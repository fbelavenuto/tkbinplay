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

#pragma once

#include "machine.h"

// Class
class apple2 : public machine {
    private:

    public:
        apple2(unsigned int sampleRate, unsigned int spb) :
            machine(sampleRate, spb) {};
        bool genAutoLoad(char *name);
        bool playCrByte(unsigned char c);
        bool playCrBuffer (char *dados, int len);
        bool playBinCrBuffer(char *buffer, int len, int loadAddr, 
	        enum actions action, int jumpAddr, int silence);

};
