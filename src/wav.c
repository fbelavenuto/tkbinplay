/* tkbinplay - Fast cassete load for TK micro
 *
 * Copyright (C) 2014-2020  Fabio Belavenuto
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "wav.h"

// Variables
static enum WaveFormat waveType  = WF_SINE;
static unsigned int  sampleRate  = 44100;
static unsigned int  bits        = 8;
static double        volume      = 1.0;
static int           waveInv     = 0;
static unsigned int  dataSize    = 0;
static TWaveCab      waveCab;
static FILE          *fileWav    = NULL;

// Public functions

/**
 * Configura o tipo de onda e taxa de amostragem
 * to = Tipo de onda
 * ta = Taxa de Amostragem
 * bi = Bits (8 ou 16)
 * vol = Volume (de 0.0 a 1.0)
 */
void wavConfig(enum WaveFormat to, unsigned int ta, unsigned int bi, double vol, int inv) {
	waveType   = to;
	sampleRate = ta;
	bits       = bi;
	volume     = vol;
	waveInv    = inv;
}

/**
 * Gera siléncio no arquivo de saída
 * durationMs = Duração em milisegundos
 * return 0 se OK, 1 se erro
 */
int playSilence(int durationMs) {
	int    total;
	char   *buffer, v;
	size_t t, s = 0;

	if (!fileWav)
		return 1;

	total = (sampleRate * durationMs) / 1000;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	v = (bits == 8) ? 128 : 0;
	memset(buffer, v, total * t);
	s = fwrite(buffer, t, total, fileWav);
	free(buffer);
	if (s != (size_t)total)
		return 1;
	dataSize += total * t;
	return 0;
}

/**
 * Gera um tom no arquivo de saída com o formato de onda configurado
 * frequency  = frequência do tom
 * durationMs = Duração em milisegundos
 * dutyCycle  = Duty Cycle (float)
 * return 0 se OK, 1 se erro
 */
int playTone(int frequency, int durationMs, double dutyCycle) {
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
	cicloT = sampleRate / frequency;
	ciclo1_2 = cicloT * dutyCycle;
	total = cicloT * durationMs;
	t = bits / 8;
	buffer = (char *)malloc(total * t + t);
	buffer2 = (short *)buffer;

	if (bits == 8) {
		amp = 127 * volume;
	} else {
		amp = 32767 * volume;
	}

	/* Calcula angulo */
	switch (waveType) {

	case WF_SQUARE:
		ang = 0;
		break;

	case WF_SINE:
		ang = (2 * PI) / cicloT;
		break;
	}

	/* Gera buffer wave */
	c = 0;
	while (c < total) {
		switch (waveType) {

		case WF_SQUARE:
			for (i = 0; i < ciclo1_2; i++) {
				if (bits == 8) {
					buffer[c++] = (waveInv) ? 128-amp : 128+amp;
				} else {
					buffer2[c++] = (waveInv) ? -amp : amp;
				}
			}
			for (     ; i < cicloT; i++) {
				if (bits == 8) {
					buffer[c++] = (waveInv) ? 128+amp : 128-amp;
				} else {
					buffer2[c++] = (waveInv) ? amp : -amp;
				}
			}
			break;

		case WF_SINE:
			for (i = 0; i < cicloT; i++) {
				amps = (double)amp;
				sincos = (waveInv) ? -sin(ang * (double)i) : sin(ang * (double)i);
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
	dataSize += c * t;
	return 0;
}

/**
 * Cria um novo arquivo WAV
 * filename = nome do arquivo de saída
 * return 0 se OK, 1 se erro
 */
int makeWavFile(char *filename) {
	size_t s = 0;

	if (!(fileWav = fopen(filename, "wb"))) {
		return 1;
	}
	memset(&waveCab, 0, sizeof(TWaveCab));

	memcpy((char *)waveCab.groupID,  "RIFF", 4);
	waveCab.groupLength    = 0;					// Fill after
	memcpy((char *)waveCab.typeID,   "WAVE", 4);
	memcpy((char *)waveCab.formatID, "fmt ", 4);
	waveCab.formatLength   = 16;
	waveCab.wFormatTag     = WAVE_FORMAT_PCM;
	waveCab.numChannels    = 1;
	waveCab.samplesPerSec  = sampleRate;
	waveCab.bytesPerSec    = sampleRate * 1 * (bits / 8);
	waveCab.nBlockAlign    = 1 * (bits / 8);
	waveCab.bitsPerSample  = bits;
	memcpy((char *)waveCab.dataID,   "data", 4);
	waveCab.dataLength     = 0;					// Fill after
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	if (s != sizeof(TWaveCab)) {
		return 1;
	}
	dataSize = 0;
	return 0;
}

/**
 * Finaliza o arquivo WAV
 * return 0 se OK, 1 se erro
 */
int finishWaveFile() {
	size_t s = 0;
	// Fornecer dados faltantes do cabeçalho
	waveCab.dataLength = dataSize;
	waveCab.groupLength = dataSize + sizeof(TWaveCab) - 8;
	if (fseek(fileWav, 0, SEEK_SET)) {
		return 1;
	}
	s = fwrite(&waveCab, 1, sizeof(TWaveCab), fileWav);
	fclose(fileWav);
	if (s != sizeof(TWaveCab)) {
		return 1;
	}
	return 0;
}

// =============================================================================
