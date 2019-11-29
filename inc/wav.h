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

#ifndef WAV_H_
#define WAV_H_

// Defines

enum TipoOnda {
	TO_QUADRADA = 0,
	TO_SENOIDAL
};

typedef struct SWaveCab {
	unsigned char  groupID[4];		// RIFF
	unsigned int   groupLength;
	unsigned char  typeID[4];		// WAVE
	unsigned char  formatID[4];		// fmt
	unsigned int   formatLength;
	unsigned short wFormatTag;
	unsigned short numChannels;
	unsigned int   samplesPerSec;
	unsigned int   bytesPerSec;
	unsigned short nBlockAlign;
	unsigned short bitsPerSample;
	unsigned char  dataID[4];
	unsigned int   dataLength;
}__attribute__((__packed__)) TWaveCab, *PTWaveCab;

#ifndef WAVE_FORMAT_PCM
# define WAVE_FORMAT_PCM                 0x0001 /* Microsoft Corporation */
#endif


// Prototipes
void wavConfig(enum TipoOnda to, unsigned int ta, unsigned int bi, double vol, int inv);
int geraSilencio(int duracaoms);
int geraTom(int frequencia, int duracaoms, double semiciclo);
int criaWav(char *arq);
int finalizaWav();

#endif /* WAV_H_ */
