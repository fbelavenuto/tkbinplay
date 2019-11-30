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

#pragma once

#include "functions.h"

// Defines
#define TK2000_BIT0 2000
#define TK2000_BIT1 1000
#define	TK2000_CABB 720

// Structs
#pragma pack(push, 1)
struct TK2000_STKCab {
	unsigned char name[6];
	unsigned char numberOfBlocks;
	unsigned char actualBlock;
}/*__attribute__((__packed__))*/;

struct TK2000_STKEnd {
	unsigned short initialAddr;
	unsigned short endAddr;
}/*__attribute__((__packed__))*/;

struct TK2000_SCh {
	unsigned char id[2];
	unsigned short size;
}/*__attribute__((__packed__))*/;

struct TK2000_SCRCab {
	unsigned short loadAddr;
	unsigned char opcode1;
	unsigned char opcode2;
	unsigned short jumpAddr;
}/*__attribute__((__packed__))*/;
#pragma pack(pop)

// Prototipes
void tk2000_samplesPerBit(int sampleRate, unsigned int spb);
int tk2000_playBuffer(unsigned char *buffer, int len);
int tk2000_playBinario(unsigned char *dados, int len, char *name, int initialAddr);
int tk2000_playCR_byte(unsigned char c);
int tk2000_playCR_buffer (char *dados, int len);
int tk2000_playBinCR_autoload(char *name);
int tk2000_playBinCR_buffer(char *buffer, int len, int loadAddr, enum actions action, int jumpAddr, int silence);

