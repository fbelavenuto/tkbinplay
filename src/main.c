/* tkbinplay - Carga rapida para micros TKs
 *
 * Copyright 2014-2020 Fábio Belavenuto
 *
 * Este arquivo é distribuido pela Licença Pública Geral GNU.
 * Veja o arquivo "LICENSE" distribuido com este software.
 *
 * ESTE SOFTWARE NÃO OFERECE NENHUMA GARANTIA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "version.h"
#include "ini.h"
#include "wav.h"
#include "tk2000.h"

// Definições
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

struct onefile {
	char 				*filename;
	int  				chargeAddr;
	int  				callAddr;
	enum tk2000_acoes	action;
	int					silence;
};

const char *ondas[] = {"Square", "Sine"};
const char *acoes[] = {"Jump", "Call", "Nothing", "Return"};

// Variáveis
static char				tk2000_name[7];
static struct onefile	*files = NULL;
static int				countFiles = 0;


// Funções

static int handler(void* user, const char* section, const char* name, const char* value) {
	int						i, chargeAddr, callAddr, silence;
	char					c, *v1, *v2;
	static struct onefile	*tmp2;
	char					tmp[MAX_PATH], *file;
	enum					tk2000_acoes action;

	if (MATCH("general", "name")) {
		// tk2000 name
		for (i = 0; i < MIN(strlen(value), 6); i++) {
			c = value[i];
			if (c >= 'a' && c <= 'z') {
				c -= 32;
			}
			tk2000_name[i] = c;
		}
	} else if (MATCH("blocks", "entry")) {
		// entrada, formato: arquivo,inicio,action,call,silence (inicio e call em HEXA)
		v1 = strchr(value, ',');
		if (v1 == NULL) {
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, value, v1-value);		// arquivo
		file = strdup(tmp);
		++v1;
		v2 = strchr(v1, ',');
		if (v2 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v1, v2-v1);
		sscanf(tmp, "%X", &chargeAddr);		// inicio
		++v2;
		v1 = strchr(v2, ',');
		if (v1 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v2, v1-v2);
		sscanf(tmp, "%d", (int *)&action);	// action
		++v1;
		v2 = strchr(v1, ',');
		if (v2 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v1, v2-v1);
		sscanf(tmp, "%X", &callAddr);		// call
		++v2;
		memset(tmp, 0, MAX_PATH);
		strcpy(tmp, v2);
		sscanf(tmp, "%d", &silence);		// silence

		if ((tmp2 = realloc(files, (countFiles+1) * sizeof(struct onefile))) == NULL) {
			fprintf(stderr, "Error: could not allocate file #%d\n", countFiles+1);
			abort();
		}
		files = tmp2;
		files[countFiles].filename = file;
		files[countFiles].chargeAddr = chargeAddr;
		files[countFiles].callAddr = callAddr;
		files[countFiles].action = action;
		files[countFiles].silence = silence;
		++countFiles;
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

// =============================================================================
char *getext(char *filename)
{
	char stack[256], *rval;
	int i, sp = 0;

	for (i = strlen(filename) - 1; i >= 0; i--) {
		if(filename[i] == '.')
			break;
		stack[sp++] = filename[i];
	}
	stack[sp] = '\0';

	if(sp == strlen(filename) || sp == 0)
		return(NULL);

	if((rval = (char *)malloc(sp * sizeof(char))) == NULL)
		; //do error code

	rval[sp] = '\0';
	for(i=0;i<sp+i;i++)
		rval[i] = stack[--sp];

	return(rval);
}

// =============================================================================
char *loadBin(char *fileName, unsigned int *fileSize) {
	char *dados = NULL;

	FILE *fileBin = fopen(fileName, "rb");
	if (!fileBin)
		return NULL;
	fseek(fileBin, 0, SEEK_END);
	*fileSize = (unsigned int)(ftell(fileBin));
	fseek(fileBin, 0, SEEK_SET);
	dados = (char *)malloc(*fileSize);
	fread(dados, 1, *fileSize, fileBin);
	fclose(fileBin);
	return dados;
}

// =============================================================================
void usage() {
	fprintf(stderr, "tkbinplay %s\n\n", VERSION);
	fprintf(stderr, "Usage: tkbinplay {options} [input INI file]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\t-o [file] output file (.wav).\n");
	fprintf(stderr, "\t-r [44100|48000] Samplerate, 44100 default.\n");
	fprintf(stderr, "\t-b [8|16] # bits, 16 default.\n");
	fprintf(stderr, "\t-i Invert wave.\n");
	fprintf(stderr, "\t-s Make senoidal wave (default).\n");
	fprintf(stderr, "\t-q Make square wave.\n");
	fprintf(stderr, "\t-p [3|4] Samples/bit, 4 default.\n");
	fprintf(stderr, "\t-V [0-100] Volume, 100 default.\n");
	fprintf(stderr, "\t-d Increase verbose output.\n");
	fprintf(stderr, "\t-h or -? show usage.\n");
	fprintf(stderr, "\t-v show version.\n");
	fprintf(stderr, "\n");
}

// =============================================================================
int main (int argc, char **argv)
{
	int				i, c;
	int				inverse = 0, verbose = 0;
	enum TipoOnda	formaOnda = TO_SENOIDAL;
	unsigned int	taxaAmostragem = 44100;
	unsigned int    samplesPerBit = 4;
	unsigned int    bits = 16;
	double			volume = 1.0;
	char 			*outputfile = NULL;
	unsigned int    filesize = 0;
	char            *buffer;

	opterr = 1;
	while((c = getopt(argc, argv, "h?vdr:b:ip:sqo:V:")) != -1) {
		switch(c) {
			case 'h':
			case '?':
				usage();
				return 1;

			case 'v':
				fprintf(stderr, "\n%s\n\n", VERSION);
				return 1;
				break;

			case 'd':
				++verbose;
				break;

			case 'r':
				taxaAmostragem = atoi(optarg);
				if (taxaAmostragem != 44100 && taxaAmostragem != 48000) {
					usage();
					return 1;
				}
				break;

			case 'b':
				bits = atoi(optarg);
				if (bits != 8 && bits != 16) {
					usage();
					return 1;
				}
				break;

			case 'p':
				samplesPerBit = atoi(optarg);
				if (samplesPerBit != 3 && samplesPerBit != 4) {
					usage();
					return 1;
				}
				break;

			case 'i':
				inverse = 1;
				break;

			case 's':
				formaOnda = TO_SENOIDAL;
				break;

			case 'q':
				formaOnda = TO_QUADRADA;
				break;

			case 'o':
				outputfile = optarg;
				break;

			case 'V':
				i = atoi(optarg);
				if (i < 0 || i > 100) {
					usage();
					return 1;
				}
				volume = (double)i / 100;
				break;

		}
	}

	if(argc - optind != 1) {
		usage();
		return 1;
	}

	if (!outputfile) {
		outputfile = "out.wav";
	}
	if (verbose) {
		fprintf(stderr,"\n");
	}

	if (ini_parse(argv[optind], handler, NULL) < 0) {
		fprintf(stderr, "Can't load '%s'\n", argv[optind]);
		return 1;
	}

	if (verbose) {
		fprintf(stderr, "Making wave. Configs:\n");
		fprintf(stderr, "Rate = %d, Bits: %d, Inverse: %d, Wave format: %s, Volume: %.02f\n", taxaAmostragem, bits, inverse, ondas[formaOnda], volume);
	}
	tk2000_samplesPerBit(taxaAmostragem, samplesPerBit);
	wavConfig(formaOnda, taxaAmostragem, bits, volume, inverse);

	if (criaWav(outputfile)) {
		fprintf(stderr, "Error creating output file '%s'\n", outputfile);
		return 1;
	}
	tk2000_playBinarioCR_autoload(tk2000_name);

	if (verbose) {
		fprintf(stderr, "# of binaries: %d\n", countFiles);
	}
	for (i=0; i < countFiles; i++) {
		if (verbose) {
			fprintf(stderr, "Processing:\n\tfilename='%s', chargeAddr='%04X'\n\t action=%s, callAddr='%04X', silence='%d ms'\n",
					files[i].filename, files[i].chargeAddr, acoes[files[i].action], files[i].callAddr, files[i].silence);
		}

		buffer = loadBin(files[i].filename, &filesize);
		if (!buffer) {
			fprintf(stderr, "Error on making wave from file '%s'\n", files[i].filename);
		} else {
			tk2000_playBinarioCR_buffer(buffer, filesize, files[i].chargeAddr, files[i].action, files[i].callAddr, files[i].silence);
			free(buffer);
		}
		free((void *)(files[i].filename));
	}
	free((void *)files);

	if (finalizaWav()) {
		fprintf(stderr, "Error on finishing wav file\n");
		return 1;
	}

	if (verbose) {
		fprintf(stderr, "Wav file '%s' finished.\n", outputfile);
	}
	return 0;
}
