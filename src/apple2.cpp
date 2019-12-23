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
#include "apple2.h"
#include "A2AutoLoad.h"
#include "A2Cr.h"

// Structs
#pragma pack(push, 1)
struct A2_SCRCab {
	unsigned short loadAddr;
	unsigned char opcode1;
	unsigned char opcode2;
	unsigned short jumpAddr;
};
#pragma pack(pop)

// Constants
const static int freqSync = 2222;
const static int freqBit0 = 2000;
const static int freqBit1 = 1000;
const static int freqHeader = 770;

// Private functions

/*****************************************************************************/
static bool playByte(unsigned char c) {
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

/*****************************************************************************/
static bool playBuffer(const unsigned char *buffer, int len) {
	int r = 0, i;
	unsigned char cs = 0xFF;

	r |= playTone(freqHeader, 3500, 0.5);		// Header
	r |= playTone(freqSync, 1, 0.44);			// Sync
	for (i = 0; i < len; i++) {
		unsigned char c = buffer[i];
		r |= playByte(c);
		cs ^= c;
	}
	r |= playByte(cs);
	r |= playTone(100, 2, 0.5);					// Final
	return r;
}

// Public class functions

/*****************************************************************************/
bool apple2::genAutoLoad(char *name) {
	int             r = 0, i, j, k, lk, pb;
	int				bufSize, end;
	unsigned char   c = 0xFC;
	unsigned char   *buffer = NULL;

    // Prepare and play Autoload code
	buffer = (unsigned char *)malloc(3);
    bufSize = A2AUTOLOAD_LENGTH + A2CR_LENGTH + getXval(4) + 10;
	end = bufSize - 1;
    buffer[0] = end & 0xFF;
    buffer[1] = (end >> 8) & 0xFF;
	buffer[2] = 0xD5;
	r |= playBuffer(buffer, 3);
    free(buffer);

    buffer = (unsigned char *)malloc(bufSize + 1);
	memset(buffer, 0, bufSize);
    memcpy(buffer, A2AutoLoad, A2AUTOLOAD_LENGTH);
	memcpy(buffer + A2AUTOLOAD_LENGTH, A2Cr, A2CR_LENGTH);
	name[30] = 0;
    sprintf((char *)&buffer[A2AUTOLOAD_LENGTH - 30],
		"LOADING %s", name);
	pb = A2AUTOLOAD_LENGTH + A2CR_LENGTH;
	lk = 0;
	for (i = 0; i < 4; i++) {
		k = (double)(((double)getXval(i+1) - 
			(double)getXval(i)) + 0.5) / 2.0;
		k += getXval(i);
		for (j = lk; j <= k; j++) {
			buffer[pb++] = c;
		}
		lk = k + 1;
		++c;
	}
	k = getXval(4) + 10;
	for (j = lk; j < k; j++) {
		buffer[pb++] = c;
	}	
	// Make audio
	r |= playBuffer(buffer, bufSize);
    free(buffer);
	r |= playSilence(200);
	return r != 0;
}

/*****************************************************************************/
bool apple2::playCrByte(unsigned char c) {
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
bool apple2::playCrBuffer(char *data, int len) {
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
bool apple2::playBinCrBuffer(char *buffer, int len, int loadAddr, 
		enum actions action, int jumpAddr, int silence) {
	int r = 0;
	struct A2_SCRCab cab;
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
	r |= playCrBuffer((char *)&cab, sizeof(struct A2_SCRCab));
	if (buffer) {
		r |= playCrBuffer(buffer, len);
	}
	r |= playSilence(silence);

	return r != 0;
}
