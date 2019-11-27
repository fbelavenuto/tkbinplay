/*
 * wav.cpp
 *
 *  Created on: 17/04/2012
 *      Author: Fabio
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "wav.h"

/* Variáveis */
enum TipoOnda tipoOnda       = TO_QUADRADA;
unsigned int  taxaAmostragem = 44100;
unsigned int  bits           = 8;
double        volume         = 1.0;
int           invertido      = 0;
unsigned int  totalDados     = 0;
TWaveCab      waveCab;
FILE          *fileWav       = NULL;

/* Funções */

/**
 * Configura o tipo de onda e taxa de amostragem
 * to = Tipo de onda
 * ta = Taxa de Amostragem
 * bi = Bits (8 ou 16)
 * vol = Volume (de 0.0 à 1.0)
 */
void wavConfig(enum TipoOnda to, unsigned int ta, unsigned int bi, double vol, int inv) {
	tipoOnda       = to;
	taxaAmostragem = ta;
	bits           = bi;
	volume         = vol;
	invertido      = inv;
}

/**
 * Gera silêncio no arquivo de saída
 * duracaoms = Duração em milisegundos
 * return 0 se OK, 1 se erro
 */
int geraSilencio(int duracaoms) {
	int    total;
	char   *buffer, v;
	size_t t, s = 0;

	if (!fileWav)
		return 1;

	total = (taxaAmostragem * duracaoms) / 1000;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	v = (bits == 8) ? 128 : 0;
	memset(buffer, v, total * t);
	s = fwrite(buffer, t, total, fileWav);
	free(buffer);
	if (s != (size_t)total)
		return 1;
	totalDados += total * t;
	return 0;
}

/**
 * Gera um tom no arquivo de saída com o formato de onda configurado
 * frequencia = frequência do tom
 * duracaoms  = Duração em milisegundos
 * semiciclo  = valor entre 0 e 1 representando a relação de tamanho
 *             entre um semiciclo e outro
 * return 0 se OK, 1 se erro
 */
int geraTom(int frequencia, int duracaoms, double semiciclo) {
	short       amp;
	char        *buffer;
	short		*buffer2;
	long double ang, amps, sincos;
	int         t, c, i;
	int         total, cicloT, ciclo1_2;
	size_t      s = 0;

	if (!fileWav)
		return 1;

	const long double PI = acos((long double) -1);

	/* Calcula tamanho do buffer e tamanho do semi-ciclo e ciclo completo */
	cicloT = taxaAmostragem / frequencia;
	ciclo1_2 = cicloT * semiciclo;
	total = cicloT * duracaoms;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	buffer2 = (short *)buffer;

	if (bits == 8) {
		amp = 127 * volume;
	} else {
		amp = 32767 * volume;
	}

	/* Calcula angulo */
	switch (tipoOnda) {

	case TO_QUADRADA:
		ang = 0;
		break;

	case TO_SENOIDAL:
		ang = (2 * PI) / cicloT;
		break;
	}

	/* Gera buffer wave */
	c = 0;
	while (c < total) {
		switch (tipoOnda) {

		case TO_QUADRADA:
			for (i = 0; i < ciclo1_2; i++) {
				if (bits == 8) {
					buffer[c++] = (invertido) ? 128-amp : 128+amp;
				} else {
					buffer2[c++] = (invertido) ? -amp : amp;
				}
			}
			for (     ; i < cicloT; i++) {
				if (bits == 8) {
					buffer[c++] = (invertido) ? 128+amp : 128-amp;
				} else {
					buffer2[c++] = (invertido) ? amp : -amp;
				}
			}
			break;

		case TO_SENOIDAL:
			for (i = 0; i < cicloT; i++) {
				amps = (double)amp;
				sincos = (invertido) ? -sin(ang * (double)i) : sin(ang * (double)i);
				if (bits == 8) {
					buffer[c++] = (char)(amps * sincos + 128.0);
				} else {
					buffer2[c++] = (short)(amps * sincos);
				}
			}
			break;
		}
	}
	s = fwrite(buffer, t, c, fileWav);
	free(buffer);
	if (s != (size_t)c)
		return 1;
	totalDados += c * t;
	return 0;
}

/**
 * Cria um novo arquivo WAV
 * arq = nome do arquivo de saída
 * return 0 se OK, 1 se erro
 */
int criaWav(char *arq) {
	size_t s = 0;

	if (!(fileWav = fopen(arq, "wb")))
		return 1;

	memset(&waveCab, 0, sizeof(TWaveCab));

	strncpy((char *)waveCab.groupID,  "RIFF", 4);
	waveCab.groupLength    = 0;					// Não fornecido agora
	strncpy((char *)waveCab.typeID,   "WAVE", 4);
	strncpy((char *)waveCab.formatID, "fmt ", 4);
	waveCab.formatLength   = 16;
	waveCab.wFormatTag     = WAVE_FORMAT_PCM;
	waveCab.numChannels    = 1;
	waveCab.samplesPerSec  = taxaAmostragem;
	waveCab.bytesPerSec    = taxaAmostragem * 1 * (bits / 8);
	waveCab.nBlockAlign    = 1 * (bits / 8);
	waveCab.bitsPerSample  = bits;
	strncpy((char *)waveCab.dataID,   "data", 4);
	waveCab.dataLength     = 0;					// Não fornecido agora
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	if (s != sizeof(TWaveCab))
		return 1;
	totalDados = 0;
	return 0;
}

/**
 * Finaliza o arquivo WAV
 * return 0 se OK, 1 se erro
 */
int finalizaWav() {
	size_t s = 0;
	// Fornecer dados faltantes do cabeçalho
	waveCab.dataLength = totalDados;
	waveCab.groupLength = totalDados + sizeof(TWaveCab) - 8;
	if (fseek(fileWav, 0, SEEK_SET))
		return 1;
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	fclose(fileWav);
	if (s != sizeof(TWaveCab))
		return 1;
	return 0;
}

// =============================================================================
