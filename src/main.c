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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "version.h"
#include "functions.h"
#include "ini.h"
#include "wav.h"
#include "tk2000.h"

// Defines

// Enums
enum systems {
	SYSTEM_TK2000 = 0,
};

// Structs
struct entry {
	char 			*filename;
	int  			chargeAddr;
	int  			callAddr;
	enum actions	action;
	int				silence;
};

// Constants
static const char *systemStr[] = {"TK2000"};
static const unsigned int systemStrCount = sizeof(systemStr) / sizeof(const char *);
static const char *wavesStr[] = {"Square", "Sine"};
//static const unsigned int wavesStrCount =  sizeof(wavesStr) / sizeof(const char *);
static const char *actionsStr[] = {"Jump", "Call", "Nothing", "Return", "Custom"};
static const unsigned int actionStrCount = sizeof(actionsStr) / sizeof(const char *);

// Variables
static enum systems machsys = SYSTEM_TK2000;
static char			nameCas[32];
static struct entry	*entries = NULL;
static int			countEntries = 0;


// Functions

/*****************************************************************************/
static int handler(void* user, const char* section, 
			const char* name, const char* value) {
	int					i, chargeAddr, callAddr, silence, ok;
	char				c, *v1, *v2;
	static struct entry	*tmp2;
	char				tmp[MAX_PATH], *file;
	enum actions		action;

	if (MATCH("general", "system")) {
		for (i = 0; i < systemStrCount; i++) {
			if (strcmp(value, systemStr[i]) == 0) {
				machsys = i;
				return 1;
			}
		}
		return 0;
	} else if (MATCH("general", "name")) {
		// Name presented in the machine
		for (i = 0; i < MIN(strlen(value), sizeof(name)); i++) {
			c = value[i];
			if (c >= 'a' && c <= 'z') {
				c -= 32;
			}
			nameCas[i] = c;
		}
	} else if (MATCH("entries", "entry")) {
		// entrada, formato: arquivo,inicio,action,call,silence (inicio e call em HEXA)
		v1 = strchr(value, ',');
		if (v1 == NULL) {
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		if (v1 > value) {
			memcpy(tmp, value, v1-value);
		}
		file = strdup(tmp);
		++v1;
		v2 = strchr(v1, ',');
		if (v2 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v1, v2-v1);
		sscanf(tmp, "%X", &chargeAddr);		// Charge Address
		++v2;
		v1 = strchr(v2, ',');
		if (v1 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v2, v1-v2);
		ok = 0;
		for (i = 0; i < actionStrCount; i++) {
			if (strcmp(tmp, actionsStr[i]) == 0) {
				action = i;
				ok = 1;
				break;
			}
		}
		if (ok == 0) {
			fprintf(stderr, "Action not recognized: %s\n", tmp);
			return 0;
		}
		++v1;
		v2 = strchr(v1, ',');
		if (v2 == NULL) {
			free(file);
			return 0;
		}
		memset(tmp, 0, MAX_PATH);
		memcpy(tmp, v1, v2-v1);
		sscanf(tmp, "%X", &callAddr);		// call or custom
		++v2;
		memset(tmp, 0, MAX_PATH);
		strcpy(tmp, v2);
		sscanf(tmp, "%d", &silence);		// silence

		if ((tmp2 = realloc(entries, (countEntries+1) * sizeof(struct entry))) == NULL) {
			fprintf(stderr, "Error: no enough memory to add entry #%d\n", countEntries+1);
			abort();
		}
		entries = tmp2;
		entries[countEntries].filename = file;
		entries[countEntries].chargeAddr = chargeAddr;
		entries[countEntries].callAddr = callAddr;
		entries[countEntries].action = action;
		entries[countEntries].silence = silence;
		++countEntries;
	} else {
		return 0;  /* unknown section/name, error */
	}
	return 1;
}

/*****************************************************************************/
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

/*****************************************************************************/
int main (int argc, char **argv) {
	int				i, c;
	int				inverse = 0, verbose = 0;
	enum TipoOnda	formaOnda = TO_SENOIDAL;
	unsigned int	taxaAmostragem = 44100;
	unsigned int    samplesPerBit = 4;
	unsigned int    bits = 16;
	double			volume = 1.0;
	char			*iniPath, filename[MAX_PATH];
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

	if (verbose) {
		fprintf(stderr,"\n");
	}

	if (ini_parse(argv[optind], handler, NULL) < 0) {
		fprintf(stderr, "Can't load '%s'\n", argv[optind]);
		return 1;
	}
	iniPath = onlyPath(argv[optind]);
	printf("%s\n", iniPath);

	if (!outputfile) {
		outputfile = withoutExt(argv[optind]);
		strcat(outputfile, ".wav");
	}
	if (verbose) {
		fprintf(stderr, "Making wave. Configs:\n");
		fprintf(stderr, 
			"System: %s, Rate: %d, Bits: %d, Inverse: %d, Wave format: %s, Volume: %.02f\n",
		 	systemStr[machsys], taxaAmostragem, bits, inverse, wavesStr[formaOnda], volume);
	}
	tk2000_samplesPerBit(taxaAmostragem, samplesPerBit);
	wavConfig(formaOnda, taxaAmostragem, bits, volume, inverse);

	if (criaWav(outputfile)) {
		fprintf(stderr, "Error creating output file '%s'\n", outputfile);
		return 1;
	}
	tk2000_playBinarioCR_autoload(nameCas);

	if (verbose) {
		fprintf(stderr, "# of entries: %d\n", countEntries);
	}
	if (countEntries == 0) {
		fprintf(stderr, "Error: No entries to process!\n");
		return 1;
	}
	for (i=0; i < countEntries; i++) {
		if (verbose) {
			fprintf(stderr, "Processing:\n\tfilename='%s', chargeAddr='%04X'\n\t action=%s, callAddr='%06X', silence='%d ms'\n",
					entries[i].filename, entries[i].chargeAddr, actionsStr[entries[i].action], entries[i].callAddr, entries[i].silence);
		}
		buffer = NULL;
		if (strlen(entries[i].filename) > 0) {
			sprintf(filename, "%s%s", iniPath, entries[i].filename);
			buffer = loadBin(filename, &filesize);
			if (!buffer) {
				fprintf(stderr, "Error reading file '%s'\n", entries[i].filename);
			}
		}
		tk2000_playBinarioCR_buffer(buffer, filesize, entries[i].chargeAddr, entries[i].action, entries[i].callAddr, entries[i].silence);
		if (buffer) {
			free(buffer);
		}
		free((void *)(entries[i].filename));
	}
	free((void *)entries);
	free(iniPath);

	if (finalizaWav()) {
		fprintf(stderr, "Error on finishing wav file\n");
		return 1;
	}

	if (verbose) {
		fprintf(stderr, "Wav file '%s' finished.\n", outputfile);
	}
	return 0;
}
