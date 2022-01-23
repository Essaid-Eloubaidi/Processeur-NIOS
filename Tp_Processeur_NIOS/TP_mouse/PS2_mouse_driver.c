#include "PS2_mouse_driver.h"
#include "nios2_ctrl_reg_macros.h"

#define DRIVER_STATE_RESET			0
#define DRIVER_STATE_ENABLE			1
#define DRIVER_STATE_RECEIVE_DATA	2

#define VIDEO_FRONT_BUFFER_ADDR		0x08000000

/**************************************************************************************/
/*****   Définition du format de la souris    ****************************************/
/**************************************************************************************/

char mouse_shape[] = {	0, -1, -1, -1, -1, -1, -1, -1,
						0,  0, -1, -1, -1, -1, -1, -1,
						0,  1,  0, -1, -1, -1, -1, -1,
						0,  1,  1,  0, -1, -1, -1, -1,
						0,  1,  1,  1,  0, -1, -1, -1,
						0,  1,  1,  1,  1,  0, -1, -1,
						0,  1,  1,  1,  1,  1,  0, -1,
						0,  1,  1,  1,  1,  0,  0,  0,
					    0,  1,  1,  1,  0, -1, -1, -1,
					    0,  0,  0,  1,  0, -1, -1, -1,
					    0, -1,  0,  1,  0, -1, -1, -1,
					   -1, -1, -1,  0,  1,  0, -1, -1, 
					   -1, -1, -1,  0,  1,  0, -1, -1,
					   -1, -1, -1, -1,  0,  1,  0, -1,
					   -1, -1, -1, -1,  0,  1,  0, -1,
					   -1, -1, -1, -1, -1,  0, -1, -1 };

unsigned short int mouse_buffer[8*16];


volatile int *PS2_ptr = (int *) 0x10000100;		// Adresse I/Os PS/2 

int mouse_x_change, mouse_y_change, mouse_x_position, mouse_y_position;
int mouse_max_x;
int mouse_max_y;
char mouse_buttons;
char is_driver_active = 0;
char is_mouse_visible = 0;
char mouse_driver_state = DRIVER_STATE_RESET;

// These values are only accessed by the ISR, so no need to make them volatile. 
char byte1, byte2, byte3;


void draw_mouse();
void erase_mouse();


void initialize_mouse_driver()
/* Initialization du pilote souris */
{
	unsigned int current_interrupt_mask = 0;

	mouse_driver_state = DRIVER_STATE_RESET;
	set_mouse_bounds(319,239);
	mouse_x_position = 160;
	mouse_y_position = 120;

	
	*(PS2_ptr + 1) = 0x1;
	NIOS2_READ_IENABLE(current_interrupt_mask);
	NIOS2_WRITE_IENABLE( current_interrupt_mask | 0x80 ); 
	NIOS2_WRITE_STATUS( 1 );		
	*(PS2_ptr) = 0xFF;
	is_driver_active = 1;
	draw_mouse();
}


void deactivate_mouse_driver()
{
	unsigned int current_interrupt_mask = 0;
	
	*(PS2_ptr + 1) = 0;
	NIOS2_READ_IENABLE(current_interrupt_mask);
	NIOS2_WRITE_IENABLE( current_interrupt_mask & 0xFFFFFF7F ); 
	is_driver_active = 0;
	erase_mouse();
}


void draw_mouse()
/* Dessiner la souris sur l'écran */
{
	int x,y;
	volatile unsigned short int *memory_buffer = (unsigned short int *) VIDEO_FRONT_BUFFER_ADDR;

	for( y = 0; y < 16; y++)
	{
		for (x = 0; x < 8; x++)
		{
			int addr = (y << 3) + x;
			int final_x = mouse_x_position + x;
			int final_y = mouse_y_position + y;

			if ((final_y < 240) && (final_x < 320))
			{
				mouse_buffer[addr] = memory_buffer[ (final_y << 9) + final_x ];
				if (mouse_shape[addr] == 0)
				{
					memory_buffer[ (final_y << 9) + final_x ] = 0;
				}
				else if (mouse_shape[addr] == 1)
				{
					memory_buffer[ (final_y << 9) + final_x ] = 0xffff;
				}
			}
		}
	}
}

void erase_mouse()
{
	int x,y;
	volatile unsigned short int *memory_buffer = (unsigned short int *) VIDEO_FRONT_BUFFER_ADDR;

	for( y = 0; y < 16; y++)
	{
		for (x = 0; x < 8; x++)
		{
			int addr = (y << 3) + x;
			int final_x = mouse_x_position + x;
			int final_y = mouse_y_position + y;
			if ((final_y < 240) && (final_x < 320))
			{
				memory_buffer[ (final_y << 9) + final_x ] = mouse_buffer[addr];
			}
		}
	}
}


void get_mouse_change(int *delta_x, int *delta_y, char *buttons)
/* Obtenir l'état de la souris */
{
	*delta_x = mouse_x_change;
	*delta_y = mouse_y_change;
	*buttons = mouse_buttons;
}


void update_mouse_coordinates(int delta_x, int delta_y)
/* Mettre à jour la position de la souris en fonction des modifications x et y reçues du périphérique de la souris. */
{
	mouse_x_position += delta_x;
	mouse_y_position -= delta_y;

	if (mouse_x_position < 0)
	{
		mouse_x_position = 0; 
	}
	if (mouse_x_position > mouse_max_x)
	{
		mouse_x_position = mouse_max_x; 
	}
	if (mouse_y_position > mouse_max_y)
	{
		mouse_y_position = mouse_max_y; 
	}
	if (mouse_y_position < 0)
	{
		mouse_y_position = 0; 
	}
}


int set_mouse_bounds(int max_x, int max_y)
/* Définir la plage maximale x / y pour la souris */
{
	int result = 0;
	if ((max_x > 0) && (max_y > 0))
	{
		mouse_max_x = max_x;
		mouse_max_y = max_y;
		update_mouse_coordinates(0, 0);
		result = 1;
	}
	return result;
}


void get_mouse_state(int *x, int *y, char *buttons)
/* Obtenir le statut de la souris. */
{
	*x = mouse_x_position;
	*y = mouse_y_position;
	*buttons = mouse_buttons;
}

void PS2_ISR( void )
{
	unsigned int PS2_data, RAVAIL;

	PS2_data = *(PS2_ptr);							
	RAVAIL = (PS2_data & 0xFFFF0000) >> 16;			

	
	if (RAVAIL > 0)
	{
		byte1 = byte2;
		byte2 = byte3;
		byte3 = PS2_data & 0x000000FF;
		switch (mouse_driver_state)
		{
			case DRIVER_STATE_RESET:
				if ( (byte1 == (char) 0xFA) && (byte2 == (char) 0xAA) && (byte3 == (char) 0x00) )
				{
					mouse_driver_state = DRIVER_STATE_ENABLE;
					*(PS2_ptr) = 0xF4;
					byte1=byte2=byte3=0;
				}
				else if ( (byte2 == (char) 0xAA) && (byte3 == (char) 0x00) )
				{
					*(PS2_ptr) = 0xFF;
					byte1=byte2=byte3=0;
				}
				break;
			case DRIVER_STATE_ENABLE:
	
				if (byte3 == (char) 0xFA)
				{
					mouse_driver_state = DRIVER_STATE_RECEIVE_DATA;
				}
				else
				{
					mouse_driver_state = DRIVER_STATE_RESET;
					*(PS2_ptr) = 0xFF;
				}
				byte1=byte2=byte3=0;
				break;
			case DRIVER_STATE_RECEIVE_DATA:
				if ((byte2 == (char) 0xAA) && (byte3 == (char) 0x00))
				{
					mouse_driver_state = DRIVER_STATE_ENABLE;
					*(PS2_ptr) = 0xF4;
					byte1=byte2=byte3=0;
				}
				else
				if ((byte1 & 0x08) != 0)
				{
					mouse_x_change = ((int)byte2) & 0x000000FF;
					if (byte1 & 0x10) mouse_x_change |= 0xFFFFFF00;
					mouse_y_change = ((int)byte3) & 0x000000FF;
					if (byte1 & 0x20) mouse_y_change |= 0xFFFFFF00;
					mouse_buttons = byte1 & 0x07;
					erase_mouse();
					update_mouse_coordinates(mouse_x_change, mouse_y_change);
					draw_mouse();

					byte1=byte2=byte3=0;
				}
				break;
			default:
				mouse_driver_state = DRIVER_STATE_RESET;
				*(PS2_ptr) = 0xFF;
				byte1=byte2=byte3=0;
				break;
		}
	}
}
