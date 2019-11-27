/*
 * tk2000.cpp
 *
 *  Created on: 24/04/2012
 *      Author: fabio
 */

#define DEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "wav.h"
#include "tk2000.h"

#include "tk2000_ar.h"
#include "tk2000_cr.h"

//const int tk2000_crfreq[4] = { 14700, 8820, 6300, 4900 }; // 14000, 8800, 6300, 4900
//const int tk2000_crfreq[4] = { 17640, 17640, 17640, 17640 };	// 2-5		2-5
//const int tk2000_crfreq[4] = { 14700, 14700, 14700, 14700 };	// 3-4		3-4
//const int tk2000_crfreq[4] = { 12600, 12600, 12600, 12600 };	// 2-4		3-4
//const int tk2000_crfreq[4] = { 11025, 11025, 11025, 11025 };	// 5-6		5-7
//const int tk2000_crfreq[4] = { 9800, 9800, 9800, 9800 };		// 5-6		5-7
//const int tk2000_crfreq[4] = { 8820, 8820, 8820, 8820 };		// 8-9		8-9
//const int tk2000_crfreq[4] = { 8018, 8018, 8018, 8018 };		// 8-9		8-9
//const int tk2000_crfreq[4] = { 7350, 7350, 7350, 7350 };		// 10-11	10-12
//const int tk2000_crfreq[4] = { 6785, 6785, 6785, 6785 };		// 10-11	11-12
//const int tk2000_crfreq[4] = { 6300, 6300, 6300, 6300 };		// 13-14	13-14
//const int tk2000_crfreq[4] = { 5880, 5880, 5880, 5880 };		// 13-14	13-14
//const int tk2000_crfreq[4] = { 5512, 5512, 5512, 5512 };		// 15-17	15-17
//const int tk2000_crfreq[4] = { 5188, 5188, 5188, 5188 };		// 16		15-17
//const int tk2000_crfreq[4] = { 4900, 4900, 4900, 4900 };		// 18-19	18-20
//const int tk2000_crfreq[4] = { 4642, 4642, 4642, 4642 };		// 18-19	18-21
//const int tk2000_crfreq[4] = { 4410, 4410, 4410, 4410 };		// 21-22	21-22
//const int tk2000_crfreq[4] = { 4200, 4200, 4200, 4200 };		// 21-22	21-22

unsigned int samplesPerBit = 4;
int tk2000_crfreq[5] = { 11025, 7350, 5512, 4410, 3675 };	// spb=4 sr=44100
int tk2000_crXval[5] = { 5, 10, 15, 20, 26 };				// spb=4 sr=44100


/**
 * Configura samples per bit
 */
void tk2000_samplesPerBit(int sampleRate, unsigned int spb) {
	int i, x = spb;
	double f;

	samplesPerBit = spb;
	for (i = 0; i < 5; i++) {
		tk2000_crfreq[i] = sampleRate / x;
		x += 2;
		// =(us-56)/9+2
		f = ((1.0 / (double)(tk2000_crfreq[i])) * 1000000.0 - 56.0) / 8.6 + 2.0;
		tk2000_crXval[i] = f;
#if DEBUG == 1
		fprintf(stderr, "%d\t%d\t%02X\n", tk2000_crfreq[i], tk2000_crXval[i], tk2000_crXval[i]);
#endif
	}

}

/**
 * Gera áudio de um byte na velocidade padrão do TK2000
 * c = byte a ter "tocado"
 */
int tk2000_playByte(unsigned char c) {
	int r = 0;
	unsigned char mask;

	for(mask = 0x80; mask; mask >>= 1) {
		if (c & mask)
			r |= geraTom(TK2000_BIT1, 1, 0.5);
		else
			r |= geraTom(TK2000_BIT0, 1, 0.5);
	}
	return r;
}

/**
 * Gera áudio do buffer
 * buffer = ponteiro do início do buffer
 * len = comprimento do buffer
 */
int tk2000_playBuffer(unsigned char *buffer, int len) {
	int r = 0, i;

	r |= geraTom(TK2000_CABB, 30, 0.5);			// Cabeçalho B
	r |= geraTom(TK2000_BIT0, 1, 0.5);			// Sincronismo
	for (i = 0; i < len; i++) {
		unsigned char c = buffer[i];
		r |= tk2000_playByte(c);
	}
	return r;
}

/**
 * Aloca uma estrutura TK2000_STKCab e a preenche
 * nome = nome do arquivo que aparecerá no tk2000, máximo de 6 caracteres
 * total_blocos = total de blocos do arquivo
 * bloco_atual = bloco atual do arquivo
 * return Ponteiro da estrutura alocada e preenchida
 */
struct TK2000_STKCab *tk2000_makeCab(char *nome, int total_blocos, int bloco_atual) {
	unsigned int i;
	struct TK2000_STKCab *dh = (struct TK2000_STKCab *)malloc(sizeof(struct TK2000_STKCab));
	memset(dh, 0, sizeof(struct TK2000_STKCab));
	memset(&dh->nome, 0xA0, 6);
	for (i=0; i < MIN(strlen(nome), 6); i++) {
		dh->nome[i] = nome[i] | 0x80;
	}
	dh->totalBlocos = total_blocos;
	dh->blocoAtual = bloco_atual;
	return dh;
}

/**
 * Aloca buffer de dados com cabeçalho e checksum
 * dh    = cabeçalho já criado e preenchido
 * dados = buffer de entrada
 * len = tamanho do buffer de entrada
 * return Buffer alocado com cabecalho e checksum
 */
char *tk2000_makeDataBlock(struct TK2000_STKCab *dh, unsigned char *dados, int len) {
	int i, t = sizeof(struct TK2000_STKCab);
	char *out = (char *)malloc(t + len + 1);
	unsigned char cs = 0xFF;

	memcpy(out, dh, t);
	memcpy(out+t, dados, len);
	for (i = 0; i < (t+len); i++) {
		cs ^= (unsigned char)(out[i]);
	}
	out[t+len] = (unsigned char)cs;
	return out;
}

/**
 * Cria cabecalho de código binário e gera áudio do cabecalho e dados
 * dados = buffer contendo o código binário
 * len = tamanho do buffer
 * nome = nome que aparecerá no tk2000 ao carregar
 * endInicial = endereço inicial de carga do código binário
 * return 0 se OK, 1 se erro
 */
int tk2000_playBinario(unsigned char *dados, int len, char *nome, int endInicial) {
	struct TK2000_STKCab *dh;
	struct TK2000_STKEnd de;
	char *buffer = NULL;
	int  r = 0, tb, ba, t1, t2;

	if (len < 1)
		return 1;
	t1 = sizeof(struct TK2000_STKCab);
	t2 = sizeof(struct TK2000_STKEnd);
	tb = ((len-1) / 256)+1;					// Calcula quantos blocos são necessários
	dh = tk2000_makeCab(nome, tb, 0);		// Cria primeiro bloco com informações
	de.endInicial = endInicial;
	de.endFinal = endInicial + len - 1;
	buffer = tk2000_makeDataBlock(dh, (unsigned char *)&de, t2);
	r |= geraSilencio(100);
	r |= geraTom(TK2000_BIT1, 1000, 0.5);		// Piloto
	r |= tk2000_playBuffer((unsigned char *)buffer, t1 + t2 + 1);
	free(buffer);
	for (ba = 1; ba <= tb; ba++) {
		dh->blocoAtual = ba;
		unsigned char *p = dados + (ba-1) * 256;
		int t3 = MIN(256, len);
		len -= 256;
		buffer = tk2000_makeDataBlock(dh, p, t3);
		r |= tk2000_playBuffer((unsigned char *)buffer, t1 + t3 + 1);
		free(buffer);
	}
	free(dh);
	r |= geraTom(100, 2, 0.5);					// Final
	r |= geraSilencio(200);
	return r;
}

// =============================================================================
int tk2000_playCR_byte(unsigned char c) {
	// Manda 2 bits por vez
	unsigned char mask = 0xC0;		// MSB primeiro
	int z, r = 0;

	for (z = 0; z < 4; z++) {
		unsigned b = (c & mask) >> (6-z*2);
		int f = tk2000_crfreq[b];
		r |= geraTom(f, 1, 0.5);
		mask >>= 2;
	}
	return r;
}

// =============================================================================
int tk2000_playCR_buffer (char *dados, int len) {
	unsigned char cs = 0;
	int i, r = 0;;

	r |= geraTom(5000, 500, 0.5);		/* Piloto */
	r |= geraTom(10000, 2, 0.5);			/* Sync */

	for (i = 0; i < len; i++) {
		unsigned char c = dados[i];
		r |= tk2000_playCR_byte(c);
		cs ^= c;
	}
	r |= tk2000_playCR_byte(cs);
	r |= geraTom(3500, 2, 0.5);			/* Final */
	return r;
}

// =============================================================================
int tk2000_playBinarioCR_autoload(char *nome) {
	int           r = 0, i, j, k, lk, pb, bufsize;
	unsigned char c = 0xFC;
	unsigned char *buffer = NULL;

	r |= tk2000_playBinario((unsigned char *)tk2000_autoload, sizeof(tk2000_autoload), nome, 0x0036);
	
	bufsize = sizeof(tk2000_cr) + tk2000_crXval[4] + 10;
	buffer = (unsigned char *)malloc(bufsize + 1);
	memcpy(buffer, tk2000_cr, sizeof(tk2000_cr));
	pb = sizeof(tk2000_cr);
	lk = 0;
	for (i=0; i < 4; i++) {
		k = (double)(((double)tk2000_crXval[i+1] - (double)tk2000_crXval[i]) + 0.5) / 2.0;
		k += tk2000_crXval[i];
		for (j=lk; j <= k; j++) {
			buffer[pb++] = c;
#if DEBUG == 1
			if (j == tk2000_crXval[i]) {
				fprintf(stderr, "[%02X],", c);
			} else {
				fprintf(stderr, "%02X,", c);
			}
#endif
		}
		lk = k+1;
		++c;
#if DEBUG == 1
		fprintf(stderr, "\n");
#endif
	}
	k = tk2000_crXval[4] + 10;
	for (j=lk; j < k; j++) {
		buffer[pb++] = c;
#if DEBUG == 1
		fprintf(stderr, "%02X,", c);
#endif
	}
#if DEBUG == 1
	fprintf(stderr, "\n");
#endif
	
	r |= tk2000_playBinario(buffer, bufsize, nome, 0x0300);
	return r;
}

// =============================================================================
int tk2000_playBinarioCR_buffer(char *buffer, int len, int endCarga, enum tk2000_acoes acao, int endJump, int silence) {
	int r = 0;
	struct TK2000_SCRCab cab;

	/* Gerar cabecalho */
	cab.endCarga = (unsigned short)endCarga;
	cab.opcode1 = 0xEA;
	if (acao == ACAO_TK2000_NADA) {
		cab.opcode2 = 0xEA;			/* NOP */
		cab.endJump = 0xEAEA;		/* NOP NOP */
	} else if (acao == ACAO_TK2000_RETURN) {
		cab.opcode2 = 0x60;			/* RTS */
	} else {
		if (acao == ACAO_TK2000_JUMP)
			cab.opcode2 = 0x4C;		/* JMP xxxx */
		else if (acao == ACAO_TK2000_CALL)
			cab.opcode2 = 0x20;		/* JSR xxxx */
		cab.endJump = endJump;
	}
	r |= tk2000_playCR_buffer((char *)&cab, sizeof(struct TK2000_SCRCab));
	r |= tk2000_playCR_buffer(buffer, len);
	r |= geraSilencio(silence);

	return r;
}


/**************************************************************************/
