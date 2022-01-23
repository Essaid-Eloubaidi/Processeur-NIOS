/**************************************************************************************/
/*****   Mouse Driver Functions    ****************************************************/
/**************************************************************************************/

void initialize_mouse_driver();
void deactivate_mouse_driver();
void get_mouse_change(int *delta_x, int *delta_y, char *buttons);
int set_mouse_bounds(int max_x, int max_y);
void get_mouse_state(int *x, int *y, char *buttons);

