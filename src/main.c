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
static const unsigned int wavesStrCount =  sizeof(wavesStr) / sizeof(const char *);
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
		// entry: file, start*, action, call*, silence(ms)
		// *in HEXA
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
	fprintf(stderr, "\t-a Do not add autoload code.\n");
	fprintf(stderr, "\t-o [file] output file (.wav).\n");
	fprintf(stderr, "\t-r [44100|48000] Samplerate, 44100 default.\n");
	fprintf(stderr, "\t-b [8|16] # bits, 16 default.\n");
	fprintf(stderr, "\t-i Invert wave polarity.\n");
	fprintf(stderr, "\t-w [square|sine] Format of output wave, sine default.\n");
	fprintf(stderr, "\t-p [3|4] Samples/bit, 4 default.\n");
	fprintf(stderr, "\t-v [0-100] Volume, 100 default.\n");
	fprintf(stderr, "\t-d Increase verbose output.\n");
	fprintf(stderr, "\t-h or -? show usage.\n");
	fprintf(stderr, "\t-V show version.\n");
	fprintf(stderr, "\n");
}

/*****************************************************************************/
int main (int argc, char **argv) {
	int				i, c, found;
	int				inverse = 0, verbose = 0, autoload = 1;
	enum WaveFormat	waveFormat = WF_SINE;
	unsigned int	sampleRate = 44100;
	unsigned int    samplesPerBit = 4;
	unsigned int    bits = 16;
	double			volume = 1.0;
	char			*iniPath, filename[MAX_PATH];
	char 			*outputfile = NULL;
	unsigned int    filesize = 0;
	char            *buffer;

	opterr = 1;
	while((c = getopt(argc, argv, "ao:r:b:iw:p:v:dh?V")) != -1) {
		switch(c) {
			case 'a':
				autoload = 0;
				break;

			case 'o':
				outputfile = strdup(optarg);
				break;

			case 'r':
				sampleRate = atoi(optarg);
				if (sampleRate != 44100 && sampleRate != 48000) {
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

			case 'i':
				inverse = 1;
				break;

			case 'w':
				found = 0;
				for (i = 0; i < wavesStrCount; i++) {
					if (strcasecmp(optarg, wavesStr[i]) == 0) {
						found = 1;
						break;
					}
				}
				if (found == 0) {
					usage();
					return 1;
				}
				waveFormat = i;
				break;

			case 'p':
				samplesPerBit = atoi(optarg);
				if (samplesPerBit != 3 && samplesPerBit != 4) {
					usage();
					return 1;
				}
				break;

			case 'v':
				i = atoi(optarg);
				if (i < 0 || i > 100) {
					usage();
					return 1;
				}
				volume = (double)i / 100;
				break;

			case 'd':
				++verbose;
				break;

			case 'h':
			case '?':
				usage();
				return 1;

			case 'V':
				fprintf(stderr, "\n%s\n\n", VERSION);
				return 1;
				break;

		}
	}

	if(argc - optind != 1) {
		usage();
		return 1;
	}

	if (ini_parse(argv[optind], handler, NULL) < 0) {
		fprintf(stderr, "Can't load '%s'\n", argv[optind]);
		return 1;
	}
	iniPath = onlyPath(argv[optind]);

	if (outputfile == NULL) {
		char *t = withoutExt(argv[optind]);
		char *p = t;
		p += strlen(iniPath);
		outputfile = (char *)malloc(strlen(p) + 5);
		strcpy(outputfile, p);
		strcat(outputfile, ".wav");
		free(t);
	}
	if (verbose) {
		fprintf(stderr,
			"System: %s, Rate: %d, Bits: %d, SPB: %d, Inverse: %d, Wave format: %s, Volume: %.02f\n",
		 	systemStr[machsys], sampleRate, bits, samplesPerBit, inverse, wavesStr[waveFormat], volume);
	}
	tk2000_samplesPerBit(sampleRate, samplesPerBit);
	wavConfig(waveFormat, sampleRate, bits, volume, inverse);

	if (verbose) {
		fprintf(stderr, "Count of entries: %d\n", countEntries);
	}
	if (countEntries == 0) {
		fprintf(stderr, "Error: No entries to process!\n");
		return 1;
	}
	if (verbose) {
		fprintf(stderr, "Creating output file '%s'.\n", outputfile);
	}
	printf("%s\n", filename); fflush(stdout);
	if (makeWavFile(outputfile)) {
		fprintf(stderr, "Error creating output file '%s'\n", outputfile);
		return 1;
	}
	if (autoload == 1) {
		tk2000_playBinCR_autoload(nameCas);
	}

	for (i = 0; i < countEntries; i++) {
		buffer = NULL;
		strcpy(filename, entries[i].filename);
		if (strlen(filename) > 0) {
			buffer = loadBin(filename, &filesize);
			if (!buffer) {
				sprintf(filename, "%s%s", iniPath, entries[i].filename);
				buffer = loadBin(filename, &filesize);
			}
			if (!buffer) {
				fprintf(stderr, "Error reading file '%s'\n", entries[i].filename);
				break;
			}
		}
		if (verbose) {
			fprintf(stderr, "Entry %d: ", i+1);
			fprintf(stderr, "Charge Addr=%04X, Action=%s, ",
					entries[i].chargeAddr, actionsStr[entries[i].action]);
			if (entries[i].action == ACTION_CUSTOM) {
				fprintf(stderr, "Custom Opcodes=%06X, ", entries[i].callAddr);
			} else {
				fprintf(stderr, "Call Addr=%04X, ", entries[i].callAddr);
			}
			fprintf(stderr, "silence=%d ms\n", entries[i].silence);
		}
		tk2000_playBinCR_buffer(buffer, filesize, entries[i].chargeAddr, entries[i].action, entries[i].callAddr, entries[i].silence);
		if (buffer) {
			free(buffer);
		}
		free((void *)(entries[i].filename));
	}

	if (finishWaveFile()) {
		fprintf(stderr, "Error on finishing wav file\n");
		return 1;
	}

	free((void *)entries);
	free(iniPath);
	free(outputfile);
	return 0;
}
