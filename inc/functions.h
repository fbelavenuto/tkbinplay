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

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

// Defines
#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n) == 0)
#ifndef MAX_PATH
# define MAX_PATH 260
#endif

// Enums
enum actions {
	ACTION_JUMP = 0,
	ACTION_CALL,
	ACTION_NOTHING,
	ACTION_RETURN,
	ACTION_CUSTOM
};

// Prototipes
char *withoutExt(const char *s);
char *onlyPath(const char *s);
//char *getExt(char *filename);
char *loadBin(char *fileName, unsigned int *fileSize);

#endif /* FUNCTIONS_H_ */