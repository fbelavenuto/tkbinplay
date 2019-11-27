/* http://srecord.sourceforge.net/ */
const unsigned char tk2000_cr[] =
{
0x4C, 0x9B, 0x03, 0x8D, 0x57, 0x03, 0x8C, 0x58, 0x03, 0xA0, 0x00, 0x84,
0xF1, 0xA9, 0xFF, 0xA2, 0x00, 0xE8, 0x2C, 0x10, 0xC0, 0x10, 0xFA, 0xE8,
0x2C, 0x10, 0xC0, 0x30, 0xFA, 0xE0, 0x16, 0x2A, 0xE0, 0x10, 0xB0, 0xEB,
0xC9, 0x00, 0xD0, 0xE5, 0x2C, 0x10, 0xC0, 0x10, 0xFB, 0xA9, 0x30, 0xE0,
0x0C, 0x90, 0x0A, 0xA9, 0x10, 0x8D, 0x40, 0x03, 0x2C, 0x10, 0xC0, 0x30,
0xFB, 0x2C, 0x10, 0xC0, 0x30, 0xFB, 0x8D, 0x6B, 0x03, 0x49, 0x20, 0x8D,
0x7B, 0x03, 0x38, 0xA9, 0xFE, 0x85, 0xF0, 0xA2, 0x00, 0x4C, 0x77, 0x03,
0x25, 0xF0, 0x99, 0x00, 0x00, 0x45, 0xF1, 0x85, 0xF1, 0xC8, 0xD0, 0x03,
0xEE, 0x58, 0x03, 0xA9, 0xFE, 0x85, 0xF0, 0xE8, 0x2C, 0x10, 0xC0, 0x30,
0xFA, 0x38, 0x26, 0xF0, 0x26, 0xF0, 0xBD, 0xB3, 0x03, 0xA2, 0x00, 0xE8,
0x2C, 0x10, 0xC0, 0x10, 0xFA, 0x90, 0xD5, 0x25, 0xF0, 0x85, 0xF0, 0x08,
0x28, 0x08, 0x28, 0xD0, 0xDE, 0xA5, 0xF1, 0xD0, 0x01, 0x60, 0x20, 0x2D,
0xFF, 0x4C, 0x2B, 0xC7, 0x00, 0x00, 0xEA, 0x6C, 0x94, 0x03, 0xEA, 0xA9,
0x94, 0xA0, 0x03, 0x20, 0x03, 0x03, 0xAD, 0x94, 0x03, 0xAC, 0x95, 0x03,
0x20, 0x03, 0x03, 0xA9, 0xEA, 0x8D, 0x9A, 0x03, 0x4C, 0x96, 0x03,
};
const unsigned long tk2000_cr_termination = 0x00000000;
const unsigned long tk2000_cr_start       = 0x00000000;
const unsigned long tk2000_cr_finish      = 0x000000B3;
const unsigned long tk2000_cr_length      = 0x000000B3;

#define TK2000_CR_TERMINATION 0x00000000
#define TK2000_CR_START       0x00000000
#define TK2000_CR_FINISH      0x000000B3
#define TK2000_CR_LENGTH      0x000000B3
