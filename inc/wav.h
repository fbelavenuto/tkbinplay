/*
 * wav.h
 *
 *  Created on: 01/05/2011
 *      Author: Fabio
 */

#ifndef WAV_H_
#define WAV_H_

// Definições

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


/* Protótipos */
void wavConfig(enum TipoOnda to, unsigned int ta, unsigned int bi, double vol, int inv);
int geraSilencio(int duracaoms);
int geraTom(int frequencia, int duracaoms, double semiciclo);
int criaWav(char *arq);
int finalizaWav();

#endif /* WAV_H_ */
