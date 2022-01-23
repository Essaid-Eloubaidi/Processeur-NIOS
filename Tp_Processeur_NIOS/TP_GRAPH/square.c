#include <stdio.h>
#include <math.h>

#define RESOLUTION_X	320
#define RESOLUTION_Y	240
//#define ABS(x)	(((x) > 0) ? (x) : -(x))

/* VGA Control registers. */
volatile int *vga_pixel_buffer_buffer_reg = (int *) 0x10003020;
volatile int *vga_pixel_buffer_back_buffer_reg = (int *) 0x10003024;

/* Video memory */
volatile int *vga_screen_front_buffer = (int *) 0x08000000;
volatile int *vga_screen_back_buffer = (int *) 0x08040000;

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

/* void draw_square(int back_buffer, int x0, int y0, int size)
{
	//register int *buffer = (back_buffer != 0) ? vga_screen_back_buffer : vga_screen_front_buffer;
	register int y,x;
	for (y = y0; y < RESOLUTION_Y && y < size; y++)
	{
		register int *buf = (buffer + (y << 8)); 
		for (x = x0 ; x < RESOLUTION_X/2 && x < size/2; x++) 
		{
			*buf++ = 0xffffffff;
		}
	}
} */

void helper_plot_pixel(int buffer_start, int x, int y, short int line_color)
{
	*((short int *)(buffer_start + (y << 10) + (x << 1))) = (short int) line_color;
}

/* Bresenham's line drawing algorithm. */
void draw_line(int x0, int y0, int x1, int y1, int color, int backbuffer)
{
	register int x_0 = x0;
	register int y_0 = y0;
	register int x_1 = x1;
	register int y_1 = y1;
//	register char steep = (ABS(y_1 - y_0) > ABS(x_1 - x_0)) ? 1 : 0;
	register int deltax, deltay, error, xstep,ystep, x, y;
	register int line_color = color;
	register unsigned int buffer_start;

	if (backbuffer == 1)
		buffer_start = (int) vga_screen_back_buffer;
	else
		buffer_start = (int) vga_screen_front_buffer;

	/* Setup local variables */
	deltax = x_1 - x_0;
//	deltay = ABS(y_1 - y_0);
	deltay = y_1 - y_0;
	int a = deltay/deltax;
	
	if (a > 1)
	{
	error = -(deltay / 2); 
	x = x_0;
	xstep=1; 

		for (y=y_0; y <= y_1; y++)
		{
			helper_plot_pixel(buffer_start, x, y, line_color);
			error = error + deltax;     
			if (error > 0) 
			{
				x = x + xstep;
				error = error - deltay;
			}
		}
	}
	else 
	{
		error = -(deltax / 2); 
	y = y_0;
	ystep=1; 

		for (x=x_0; x <= x_1; x++)
		{
			helper_plot_pixel(buffer_start, x, y, line_color);
			error = error + deltay;     
			if (error > 0) 
			{
				y = y + ystep;
				error = error - deltax;
			}
		}
	}
}

int main(void)
{
	clear_screen(0);
	draw_line(0, 0, 200, 150, 0x001f, 0);
	draw_line(640, 480, 440, 330, 0, 0);
	draw_line(0, 480, 200, 330, 0, 0);
	draw_line(640, 0, 440, 150, 0, 0);
	//draw_square (0, 50,50,100);
}
