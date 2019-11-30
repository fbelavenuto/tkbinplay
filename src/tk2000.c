/* tkbinplay - Fast cassete load for TK micro
 *
 * Copyright (C) 2014-2020  Fabio Belavenuto
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.**
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"
#include "wav.h"
#include "tk2000.h"
#include "tk2000_ar.h"
#include "tk2000_cr.h"

// Defines
#define INT_DEBUG 0

// Variables
static int tk2000_crfreq[5] = { 11025, 7350, 5512, 4410, 3675 };	// spb=4 sr=44100
static int tk2000_crXval[5] = { 5, 10, 15, 20, 26 };				// spb=4 sr=44100

// Private functions

/*****************************************************************************
 * Aloca uma estrutura TK2000_STKCab e a preenche
 * nome = nome do arquivo que aparecera no tk2000, maximo de 6 caracteres
 * numberOfBlocks = total de blocos do arquivo
 * actualBlock = bloco atual do arquivo
 * return Ponteiro da estrutura alocada e preenchida
 */
static struct TK2000_STKCab *tk2000_makeCab(char *name,
		int numberOfBlocks, int actualBlock) {
	unsigned int i;
	struct TK2000_STKCab *dh = 
		(struct TK2000_STKCab *)malloc(sizeof(struct TK2000_STKCab));
	memset(dh, 0, sizeof(struct TK2000_STKCab));
	memset(&dh->name, 0xA0, 6);
	for (i=0; i < MIN(strlen(name), 6); i++) {
		dh->name[i] = name[i] | 0x80;
	}
	dh->numberOfBlocks = numberOfBlocks;
	dh->actualBlock = actualBlock;
	return dh;
}

/*****************************************************************************
 * Aloca buffer de dados com cabecalho e checksum
 * dh    = cabecalho ja criado e preenchido
 * dados = buffer de entrada
 * len = tamanho do buffer de entrada
 * return Buffer alocado com cabecalho e checksum
 */
static char *tk2000_makeDataBlock(struct TK2000_STKCab *dh, 
		unsigned char *data, unsigned int len) {
	int i, t = sizeof(struct TK2000_STKCab);
	char *out = (char *)malloc(t + len + 1);
	unsigned char cs = 0xFF;

	memcpy(out, dh, t);
	memcpy(out+t, data, len);
	for (i = 0; i < (t+len); i++) {
		cs ^= (unsigned char)(out[i]);
	}
	out[t+len] = (unsigned char)cs;
	return out;
}

/*****************************************************************************
 * Gera audio de um byte na velocidade padrao do TK2000
 * c = byte a ter "tocado"
 */
static int tk2000_playByte(unsigned char c) {
	int r = 0;
	unsigned char mask;

	for(mask = 0x80; mask; mask >>= 1) {
		if (c & mask)
			r |= playTone(TK2000_BIT1, 1, 0.5);
		else
			r |= playTone(TK2000_BIT0, 1, 0.5);
	}
	return r;
}

// Public functions

/*****************************************************************************
 * Configura samples per bit
 */
void tk2000_samplesPerBit(int sampleRate, unsigned int spb) {
	int i, x = spb;
	double f;
	for (i = 0; i < 5; i++) {
		tk2000_crfreq[i] = sampleRate / x;
		x += 2;
		// =(us-56)/9+2
		f = ((1.0 / (double)(tk2000_crfreq[i])) * 1000000.0 - 56.0) / 8.6 + 2.0;
		tk2000_crXval[i] = f;
#if INT_DEBUG == 1
		fprintf(stderr, "%d\t%d\t%02X\n", tk2000_crfreq[i], tk2000_crXval[i], tk2000_crXval[i]);
#endif
	}
}

/*****************************************************************************
 * Gera audio do buffer
 * buffer = ponteiro do inicio do buffer
 * len = comprimento do buffer
 */
int tk2000_playBuffer(unsigned char *buffer, int len) {
	int r = 0, i;

	r |= playTone(TK2000_CABB, 30, 0.5);		// Cabecalho B
	r |= playTone(TK2000_BIT0, 1, 0.5);			// Sincronismo
	for (i = 0; i < len; i++) {
		unsigned char c = buffer[i];
		r |= tk2000_playByte(c);
	}
	return r;
}

/*****************************************************************************
 * Cria cabecalho de codigo binario e gera audio do cabecalho e dados
 * dados = buffer contendo o codigo binario
 * len = tamanho do buffer
 * nome = nome que aparecera no tk2000 ao carregar
 * initialAddr = endereco inicial de carga do codigo binario
 * return 0 se OK, 1 se erro
 */
int tk2000_playBin(unsigned char *data, int len, char *name,
		int initialAddr) {
	struct TK2000_STKCab *dh;
	struct TK2000_STKEnd de;
	char *buffer = NULL;
	int  r = 0, tb, ba, t1, t2;

	if (len < 1)
		return 1;
	t1 = sizeof(struct TK2000_STKCab);
	t2 = sizeof(struct TK2000_STKEnd);
	tb = ((len-1) / 256)+1;					// Calcula quantos blocos são necessários
	dh = tk2000_makeCab(name, tb, 0);		// Cria primeiro bloco com informações
	de.initialAddr = initialAddr;
	de.endAddr = initialAddr + len - 1;
	buffer = tk2000_makeDataBlock(dh, (unsigned char *)&de, t2);
	r |= playSilence(100);
	r |= playTone(TK2000_BIT1, 1000, 0.5);		// Piloto
	r |= tk2000_playBuffer((unsigned char *)buffer, t1 + t2 + 1);
	free(buffer);
	for (ba = 1; ba <= tb; ba++) {
		dh->actualBlock = ba;
		unsigned char *p = data + (ba-1) * 256;
		int t3 = MIN(256, len);
		len -= 256;
		buffer = tk2000_makeDataBlock(dh, p, t3);
		r |= tk2000_playBuffer((unsigned char *)buffer, t1 + t3 + 1);
		free(buffer);
	}
	free(dh);
	r |= playTone(100, 2, 0.5);					// Final
	r |= playSilence(200);
	return r;
}

/*****************************************************************************/
int tk2000_playCR_byte(unsigned char c) {
	// Manda 2 bits por vez
	unsigned char mask = 0xC0;		// MSB primeiro
	int z, r = 0;

	for (z = 0; z < 4; z++) {
		unsigned b = (c & mask) >> (6-z*2);
		int f = tk2000_crfreq[b];
		r |= playTone(f, 1, 0.5);
		mask >>= 2;
	}
	return r;
}

/*****************************************************************************/
int tk2000_playCR_buffer (char *data, int len) {
	unsigned char cs = 0;
	int i, r = 0;;

	r |= playTone(5000, 500, 0.5);		/* Piloto */
	r |= playTone(10000, 2, 0.5);			/* Sync */

	for (i = 0; i < len; i++) {
		unsigned char c = data[i];
		r |= tk2000_playCR_byte(c);
		cs ^= c;
	}
	r |= tk2000_playCR_byte(cs);
	r |= playTone(3500, 2, 0.5);			/* Final */
	return r;
}

/*****************************************************************************/
int tk2000_playBinCR_autoload(char *name) {
	int           r = 0, i, j, k, lk, pb, bufsize;
	unsigned char c = 0xFC;
	unsigned char *buffer = NULL;

	r |= tk2000_playBin((unsigned char *)tk2000_autoload, 
			sizeof(tk2000_autoload), name, 0x0036);
	
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
	
	r |= tk2000_playBin(buffer, bufsize, name, 0x0300);
	return r;
}

/*****************************************************************************/
int tk2000_playBinCR_buffer(char *buffer, int len, int loadAddr, 
		enum actions action, int jumpAddr, int silence) {
	int r = 0;
	struct TK2000_SCRCab cab;
	unsigned char *p;

	/* Gerar cabecalho */
	if (buffer) {
		cab.loadAddr = (unsigned short)loadAddr;
	} else {
		cab.loadAddr = 0;
	}
	cab.opcode1 = 0xEA;	// reserved for future
	switch(action) {
		case ACTION_JUMP:
			cab.opcode2 = 0x4C;		/* JMP xxxx */
			cab.jumpAddr = jumpAddr & 0xFFFF;
			break;

		case ACTION_CALL:
			cab.opcode2 = 0x20;		/* JSR xxxx */
			cab.jumpAddr = jumpAddr & 0xFFFF;
			break;

		case ACTION_NOTHING: 
			cab.opcode2 = 0xEA;		/* NOP */
			cab.jumpAddr = 0xEAEA;	/* NOP NOP */
			break;

		case ACTION_RETURN:
			cab.opcode2 = 0x60;		/* RTS */
			break;

		case ACTION_CUSTOM:
			p = (unsigned char *)&jumpAddr;
			cab.opcode2 = p[2];
			cab.jumpAddr = p[0] << 8 | p[1];
			break;

	}
	r |= tk2000_playCR_buffer((char *)&cab, sizeof(struct TK2000_SCRCab));
	if (buffer) {
		r |= tk2000_playCR_buffer(buffer, len);
	}
	r |= playSilence(silence);

	return r;
}


/**************************************************************************/
