
unsigned char tk2000_autoload[40] = {
		0x50, 0x00,
		0x0F, 0xFD, 0x00, 0x00,
		0x3C, 0x00,
		0x5D, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC1,		// 48 = $20 e $A0

		0xA9, 0xF0,				// LDA #$F0
		0x85, 0x36,				// STA $36
		0xA9, 0xFD,				// LDA #$FD
		0x85, 0x37,				// STA $37
		0x20, 0xAE, 0xED,		// JSR $EDAE
		0x4C, 0x00, 0x03,		// JMP $0300
};
