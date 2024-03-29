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
#include "functions.h"

/*****************************************************************************/
char *trim(const char *s) {
	char *ptr;
	if (!s)
		return NULL; // handle NULL string
	if (!*s)
		return (char *)s; // handle empty string
	for (ptr = (char *)s + strlen(s) - 1; (ptr >= s) && (*ptr == ' '); --ptr)
		;
	ptr[1] = '\0';
	return (char *)s;
}

/*****************************************************************************/
/** Make a new string without extension
 * 
 */
char *withoutExt(const char *s) {
	char *o = (char *)malloc(strlen(s) + 1);
	strcpy(o, s);
	char *p = (char *)(o + strlen(o));
	while(--p > o) {
		if (*p == '.') {
			*p = '\0';
			break;
		}
	}
	return o;
}

/*****************************************************************************/
char *onlyPath(const char *s) {
	char *p = (char *)(s + strlen(s));
	while(--p > s) {
		if (*p == '/' || *p == '\\') {
			++p;
			break;
		}
	}
	unsigned int l = p - s;
	char *o = (char *)malloc(l+1);
	memset(o, 0, l+1);
	if (l > 0) {
		memcpy(o, s, l);
	}
	return o;
}

/*****************************************************************************/
char *loadBin(const char *fileName, unsigned int *fileSize) {
	char *data = NULL;

	FILE *fileBin = fopen(fileName, "rb");
	if (!fileBin)
		return NULL;
	fseek(fileBin, 0, SEEK_END);
	*fileSize = (unsigned int)(ftell(fileBin));
	fseek(fileBin, 0, SEEK_SET);
	data = (char *)malloc(*fileSize);
	fread(data, 1, *fileSize, fileBin);
	fclose(fileBin);
	return data;
}
