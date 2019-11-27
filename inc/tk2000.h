/*
 * tk2000.h
 *
 *  Created on: 24/04/2012
 *      Author: fabio
 */

#ifndef TK2000_H_
#define TK2000_H_


/* Definições */
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define TK2000_BIT0 2000
#define TK2000_BIT1 1000
#define	TK2000_CABB 720

enum tk2000_acoes {
	ACAO_TK2000_JUMP = 0,
	ACAO_TK2000_CALL,
	ACAO_TK2000_NADA,
	ACAO_TK2000_RETURN
};

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


/* Protótipos */
void tk2000_samplesPerBit(int sampleRate, unsigned int spb);
int tk2000_playBuffer(unsigned char *buffer, int len);
int tk2000_playBinario(unsigned char *dados, int len, char *nome, int endInicial);
int tk2000_playCR_byte(unsigned char c);
int tk2000_playCR_buffer (char *dados, int len);
int tk2000_playBinarioCR_autoload(char *nome);
int tk2000_playBinarioCR_buffer(char *buffer, int len, int endCarga, enum tk2000_acoes acao, int endJump, int silence);

#endif /* TK2000_H_ */
