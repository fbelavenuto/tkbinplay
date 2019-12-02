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

unsigned char tk2kAutoLoad[40] = {
		0x50, 0x00,
		0x0F, 0xFD, 0x00, 0x00,
		0x3C, 0x00,
		0x5D, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC1,		// 48 = $20 or $A0

		0xA9, 0xF0,				// LDA #$F0
		0x85, 0x36,				// STA $36
		0xA9, 0xFD,				// LDA #$FD
		0x85, 0x37,				// STA $37
		0x20, 0xAE, 0xED,		// JSR $EDAE
		0x4C, 0x00, 0x03,		// JMP $0300
};
