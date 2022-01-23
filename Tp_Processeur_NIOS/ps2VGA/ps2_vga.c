#include "nios2_ctrl_reg_macros.h"
#include "PS2_mouse_driver.h"

#define RESOLUTION_X	320
#define RESOLUTION_Y	240

/* VGA Control registers. */
volatile int *vga_pixel_buffer_buffer_reg = (int *) 0x10003020;
volatile int *vga_pixel_buffer_back_buffer_reg = (int *) 0x10003024;

/* Video memory */
volatile int *vga_screen_front_buffer = (int *) 0x08000000;
volatile int *vga_screen_back_buffer = (int *) 0x08040000;

/****************************************************************************************
 * Afficahge HEX
 ****************************************************************************************/
void show_mouse_state(int mouse_delta_x, int mouse_delta_y, char mouse_buttons)
{
	volatile int *HEX3_HEX0_ptr = (int *) 0x10000020;
	volatile int *HEX7_HEX4_ptr = (int *) 0x10000030;
	volatile int *ledr_ptr = (int *) 0x10000000;

	unsigned char	seven_seg_decode_table[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7C, 0x07, 
		  										0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };
	unsigned char	hex_segs[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int	shift_buffer, nibble;
	unsigned char	code;
	int i;

	shift_buffer = ((mouse_delta_x << 16) & 0xFFFF0000) | (mouse_delta_y & 0x0000FFFF);

	for ( i = 0; i < 8; ++i )
	{
		nibble = shift_buffer & 0x0000000F;		// character is in rightmost nibble
		code = seven_seg_decode_table[nibble];
		hex_segs[i] = code;
		shift_buffer = shift_buffer >> 4;
	}
	*(HEX3_HEX0_ptr) = *(int *) (hex_segs);
	*(HEX7_HEX4_ptr) = *(int *) (hex_segs+4);
	*ledr_ptr = (mouse_buttons & 0x07);
}

void clear_screen(int back_buffer)
{
	register int *buffer = (back_buffer != 0) ? vga_screen_back_buffer : vga_screen_front_buffer;
	register int y,x;
	for (y = 0; y < RESOLUTION_Y; y++)
	{
		register int *buf = (buffer + (y << 8)); 
		for (x = 0; x < RESOLUTION_X/2; x++) 
		{
			*buf++ = 0xffffffff;
		}
	}
}

 void draw_square(int back_buffer, int x0, int y0, int size)
{
	register int *buffer = (back_buffer != 0) ? vga_screen_back_buffer : vga_screen_front_buffer;
	register int y,x;
	for (y = y0; y < y0 + size; y++)
	{
		register int *buf = (buffer + (y << 8) + x0/2); 
		for (x = x0 ; x < x0 + size/2 ; x++) 
		{
			*buf++ = 0x001f001f;
		}
	}
}

int main(void)
{
	volatile short int *video_memory = (short int *) 0x08000000;
	int i;

	for (i=0; i < 122880; i++)
		video_memory[i] = 0;

	initialize_mouse_driver();
	
	while(1) {
		int mouse_x, mouse_y;
		char mouse_buttons;
		clear_screen(0);
		//get_mouse_state(&mouse_x, &mouse_y, &mouse_buttons);
		//show_mouse_state(mouse_x, mouse_y, mouse_buttons);
		clear_screen(0);
		draw_square (0, 50, 50,100);
	}

	deactivate_mouse_driver();
}
