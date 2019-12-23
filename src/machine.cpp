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
#include "machine.h"


/*****************************************************************************/
machine::machine(unsigned int sampleRate, unsigned int spb) {
	int i, x = spb;
	double f;
	for (i = 0; i < 5; i++) {
		crFreqs[i] = sampleRate / x;
		x += 2;
		f = ((1.0 / (double)(crFreqs[i])) * 1000000.0 - 56.0) / 8.6 + 2.0;
		crXvals[i] = f;
	}
}

/*****************************************************************************/
machine::~machine() {

}

/*****************************************************************************/
int machine::getFreq(int i) {
	if (i < 0 || i > 4) {
		throw "Invalid index";
	}
	return crFreqs[i];
}

/*****************************************************************************/
int machine::getXval(int i) {
	if (i < 0 || i > 4) {
		throw "Invalid index";
	}
	return crXvals[i];
}
