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

#ifndef TK2000_H_
#define TK2000_H_

#include "functions.h"

// Defines
#define TK2000_BIT0 2000
#define TK2000_BIT1 1000
#define	TK2000_CABB 720

// Structs
struct TK2000_STKCab
{
	unsigned char nome[6];
	unsigned char totalBlocos;
	unsigned char blocoAtual;
}__attribute__((__packed__));

struct TK2000_STKEnd
{
	unsigned short endInicial;
	unsigned short endFinal;
}__attribute__((__packed__));

struct TK2000_SCh
{
	unsigned char id[2];
	unsigned short tam;
}__attribute__((__packed__));

struct TK2000_SCRCab
{
	unsigned short endCarga;
	unsigned char opcode1;
	unsigned char opcode2;
	unsigned short endJump;
}__attribute__((__packed__));


// Prototipes
void tk2000_samplesPerBit(int sampleRate, unsigned int spb);
int tk2000_playBuffer(unsigned char *buffer, int len);
int tk2000_playBinario(unsigned char *dados, int len, char *nome, int endInicial);
int tk2000_playCR_byte(unsigned char c);
int tk2000_playCR_buffer (char *dados, int len);
int tk2000_playBinarioCR_autoload(char *nome);
int tk2000_playBinarioCR_buffer(char *buffer, int len, int endCarga, enum actions acao, int endJump, int silence);

#endif /* TK2000_H_ */
