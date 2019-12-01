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

	r |= tk2kPlayBin((unsigned char *)tk2000_autoload, 
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
	
	r |= tk2kPlayBin(buffer, bufsize, name, 0x0300);
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
