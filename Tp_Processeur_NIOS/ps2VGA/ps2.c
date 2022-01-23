
void HEX_PS2(char b1, char b2, char b3)
{
	volatile int * HEX3_HEX0_ptr = (int *) 0x10000020;
	volatile int * HEX7_HEX4_ptr = (int *) 0x10000030;


	unsigned char	seven_seg_decode_table[] = {	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7C, 0x07,
		  										0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };
	unsigned char	hex_segs[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int shift_buffer, nibble;
	unsigned char code;
	int i;

	shift_buffer = ((b1 << 16) & 0x00FF0000) | ((b2 << 8) & 0x0000FF00)| (b3 & 0x000000FF);
	for ( i = 0; i < 6; ++i )
	{
		nibble = shift_buffer & 0x0000000F;		// character is in rightmost nibble
		code = seven_seg_decode_table[nibble];
		hex_segs[i] = code;
		shift_buffer = shift_buffer >> 4;
	}
	/* drive the hex displays */
	*(HEX3_HEX0_ptr) = *(int *) (hex_segs);
	*(HEX7_HEX4_ptr) = *(int *) (hex_segs+4);
}

/****************************************************************************************
 * Main functions
 ****************************************************************************************/

int main(void)
{
	unsigned int PS2_data, RAVAIL;
	volatile int *PS2_ptr = (int *) 0x10000100;
	char byte1=0, byte2=0, byte3=0;
	char enable = 1;

	*PS2_ptr = 0xf4;
	while (1)
	{
		PS2_data = *(PS2_ptr);
		RAVAIL = (PS2_data & 0xFFFF0000) >> 16;


		if ((RAVAIL > 0) && (enable == 1))
		{
			byte1 = byte2;
			byte2 = byte3;
			byte3 = PS2_data & 0x00FF;

			if ((byte2 == (char) 0xAA) && (byte3 == (char) 0))
			{
				enable = 0;
			}
		}
		HEX_PS2 (byte1, byte2, byte3);
	}
}
