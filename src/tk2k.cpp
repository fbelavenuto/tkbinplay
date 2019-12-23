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
#include "tk2k.h"
#include "tk2kAutoLoad.h"
#include "tk2kCr.h"

// Structs
#pragma pack(push, 1)
struct TK2000_SCRCab {
	unsigned short loadAddr;
	unsigned char opcode1;
	unsigned char opcode2;
	unsigned short jumpAddr;
};
#pragma pack(pop)

// Public class functions

/*****************************************************************************/
bool tk2k::genAutoLoad(char *name) {
	int           r = 0, i, j, k, lk, pb, bufsize;
	unsigned char c = 0xFC;
	unsigned char *buffer = NULL;

	r |= tk2kPlayBin((char *)tk2kAutoLoad, 
			sizeof(tk2kAutoLoad), name, 0x0036);
	
	bufsize = sizeof(tk2kCr) + getXval(4) + 10;
	buffer = (unsigned char *)malloc(bufsize + 1);
	memcpy(buffer, tk2kCr, sizeof(tk2kCr));
	pb = sizeof(tk2kCr);
	lk = 0;
	for (i=0; i < 4; i++) {
		k = (double)(((double)getXval(i+1) - 
			(double)getXval(i)) + 0.5) / 2.0;
		k += getXval(i);
		for (j=lk; j <= k; j++) {
			buffer[pb++] = c;
		}
		lk = k+1;
		++c;
	}
	k = getXval(4) + 10;
	for (j=lk; j < k; j++) {
		buffer[pb++] = c;
	}	
	r |= tk2kPlayBin((char *)buffer, bufsize, name, 0x0300);
	return r != 0;
}

/*****************************************************************************/
bool tk2k::playCrByte(unsigned char c) {
	// Manda 2 bits por vez
	unsigned char mask = 0xC0;		// MSB primeiro
	int z, r = 0;

	for (z = 0; z < 4; z++) {
		unsigned b = (c & mask) >> (6-z*2);
		int f = getFreq(b);
		r |= playTone(f, 1, 0.5);
		mask >>= 2;
	}
	return r != 0;
}

/*****************************************************************************/
bool tk2k::playCrBuffer(char *data, int len) {
	unsigned char cs = 0;
	int i, r = 0;;

	r |= playTone(5000, 500, 0.5);			/* Pilot */
	r |= playTone(10000, 2, 0.5);			/* Sync */

	for (i = 0; i < len; i++) {
		unsigned char c = data[i];
		r |= playCrByte(c);
		cs ^= c;
	}
	r |= playCrByte(cs);
	r |= playTone(3500, 2, 0.5);			/* End */
	return r != 0;
}

/*****************************************************************************/
bool tk2k::playBinCrBuffer(char *buffer, int len, int loadAddr, 
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
	r |= playCrBuffer((char *)&cab, sizeof(struct TK2000_SCRCab));
	if (buffer) {
		r |= playCrBuffer(buffer, len);
	}
	r |= playSilence(silence);

	return r != 0;
}
