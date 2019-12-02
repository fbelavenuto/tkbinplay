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

#include "ct2.h"
#include "functions.h"

// Structs
#pragma pack(push, 1)
struct TK2000_SCRCab {
	unsigned short loadAddr;
	unsigned char opcode1;
	unsigned char opcode2;
	unsigned short jumpAddr;
};
#pragma pack(pop)

// Prototipes
void tk2kSetSpb(int sampleRate, unsigned int spb);
int tk2kPlayBinAl(char *name);
int tk2kPlayCrByte(unsigned char c);
int tk2kPlayCrBuffer (char *dados, int len);
int tk2kPlayBinCrBuffer(char *buffer, int len, int loadAddr, 
	enum actions action, int jumpAddr, int silence);
