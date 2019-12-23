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

// Enums
enum actions {
	ACTION_JUMP = 0,
	ACTION_CALL,
	ACTION_NOTHING,
	ACTION_RETURN,
	ACTION_CUSTOM
};

// Class
class machine {
    private:
        int crFreqs[5];
        int crXvals[5];

    protected:
        machine(unsigned int sampleRate, unsigned int spb);
        ~machine();

    public:
        int getFreq(int i);
        int getXval(int i);
        virtual bool genAutoLoad(char *name) = 0;
        virtual bool playCrByte(unsigned char c) = 0;
        virtual bool playCrBuffer (char *dados, int len) = 0;
        virtual bool playBinCrBuffer(char *buffer, int len, int loadAddr, 
	        enum actions action, int jumpAddr, int silence) = 0;

};
