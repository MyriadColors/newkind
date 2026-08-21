/*
 * Elite - The New Kind (Raylib Port)
 *
 * Raylib version of keyboard input handler.
 */

#define Matrix RaylibMatrix
#include "raylib.h"
#undef Matrix
#include "keyboard.h"

int kbd_F1_pressed = 0;
int kbd_F2_pressed = 0;
int kbd_F3_pressed = 0;
int kbd_F4_pressed = 0;
int kbd_F5_pressed = 0;
int kbd_F6_pressed = 0;
int kbd_F7_pressed = 0;
int kbd_F8_pressed = 0;
int kbd_F9_pressed = 0;
int kbd_F10_pressed = 0;
int kbd_F11_pressed = 0;
int kbd_F12_pressed = 0;

int kbd_y_pressed = 0;
int kbd_n_pressed = 0;
int kbd_fire_pressed = 0;
int kbd_ecm_pressed = 0;
int kbd_energy_bomb_pressed = 0;
int kbd_hyperspace_pressed = 0;
int kbd_ctrl_pressed = 0;
int kbd_jump_pressed = 0;
int kbd_escape_pressed = 0;
int kbd_dock_pressed = 0;
int kbd_d_pressed = 0;
int kbd_origin_pressed = 0;
int kbd_find_pressed = 0;

int kbd_fire_missile_pressed = 0;
int kbd_target_missile_pressed = 0;
int kbd_unarm_missile_pressed = 0;

int kbd_pause_pressed = 0;
int kbd_resume_pressed = 0;

int kbd_inc_speed_pressed = 0;
int kbd_dec_speed_pressed = 0;

int kbd_up_pressed = 0;
int kbd_down_pressed = 0;
int kbd_left_pressed = 0;
int kbd_right_pressed = 0;

int kbd_enter_pressed = 0;
int kbd_backspace_pressed = 0;
int kbd_space_pressed = 0;

int kbd_keyboard_startup(void)
{
	return 0;
}

int kbd_keyboard_shutdown(void)
{
	return 0;
}

void kbd_poll_keyboard(void)
{
	kbd_F1_pressed = IsKeyPressed(KEY_F1);
	kbd_F2_pressed = IsKeyPressed(KEY_F2);
	kbd_F3_pressed = IsKeyPressed(KEY_F3);
	kbd_F4_pressed = IsKeyPressed(KEY_F4);
	kbd_F5_pressed = IsKeyPressed(KEY_F5);
	kbd_F6_pressed = IsKeyPressed(KEY_F6);
	kbd_F7_pressed = IsKeyPressed(KEY_F7);
	kbd_F8_pressed = IsKeyPressed(KEY_F8);
	kbd_F9_pressed = IsKeyPressed(KEY_F9);
	kbd_F10_pressed = IsKeyPressed(KEY_F10);
	kbd_F11_pressed = IsKeyPressed(KEY_F11);
	kbd_F12_pressed = IsKeyPressed(KEY_F12);

	kbd_y_pressed = IsKeyPressed(KEY_Y);
	kbd_n_pressed = IsKeyPressed(KEY_N);

	kbd_fire_pressed = IsKeyDown(KEY_A);
	kbd_ecm_pressed = IsKeyPressed(KEY_E);
	kbd_energy_bomb_pressed = IsKeyPressed(KEY_TAB);
	kbd_hyperspace_pressed = IsKeyPressed(KEY_H);
	kbd_ctrl_pressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
	kbd_jump_pressed = IsKeyPressed(KEY_J);
	kbd_escape_pressed = IsKeyPressed(KEY_ESCAPE);
	kbd_dock_pressed = IsKeyPressed(KEY_C);
	kbd_d_pressed = IsKeyPressed(KEY_D);
	kbd_origin_pressed = IsKeyPressed(KEY_O);
	kbd_find_pressed = IsKeyPressed(KEY_F);

	kbd_fire_missile_pressed = IsKeyPressed(KEY_M);
	kbd_target_missile_pressed = IsKeyPressed(KEY_T);
	kbd_unarm_missile_pressed = IsKeyPressed(KEY_U);

	kbd_pause_pressed = IsKeyPressed(KEY_P);
	kbd_resume_pressed = IsKeyPressed(KEY_R);

	kbd_inc_speed_pressed = IsKeyDown(KEY_SPACE);
	kbd_dec_speed_pressed = IsKeyDown(KEY_SLASH);

	kbd_up_pressed = IsKeyPressed(KEY_UP) || IsKeyDown(KEY_S);
	kbd_down_pressed = IsKeyPressed(KEY_DOWN) || IsKeyDown(KEY_X);
	kbd_left_pressed = IsKeyPressed(KEY_LEFT) || IsKeyDown(KEY_COMMA);
	kbd_right_pressed = IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_PERIOD);

	kbd_enter_pressed = IsKeyPressed(KEY_ENTER);
	kbd_backspace_pressed = IsKeyPressed(KEY_BACKSPACE);
	kbd_space_pressed = IsKeyPressed(KEY_SPACE);
}

int kbd_read_key(void)
{
	int key = GetCharPressed();
	if (key > 0)
		return key;

	if (IsKeyPressed(KEY_ENTER)) return '\r';
	if (IsKeyPressed(KEY_BACKSPACE)) return 8;
	if (IsKeyPressed(KEY_ESCAPE)) return 27;

	return 0;
}

void kbd_clear_key_buffer(void)
{
	while (GetCharPressed() > 0) {}
}
