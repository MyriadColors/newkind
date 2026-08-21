/*
 * Elite - The New Kind (Raylib Port)
 *
 * Raylib version of keyboard input handler.
 */

#define Matrix RaylibMatrix
#include "raylib.h"
#undef Matrix
#include "keyboard.h"
#include "elite.h"

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

int kbd_w_down = 0;
int kbd_s_down = 0;
int kbd_a_down = 0;
int kbd_d_down = 0;
int kbd_q_down = 0;
int kbd_e_down = 0;
int kbd_up_down = 0;
int kbd_down_down = 0;
int kbd_left_down = 0;
int kbd_right_down = 0;
int kbd_q_pressed = 0;
int kbd_shift_down = 0;
int kbd_lctrl_down = 0;
int kbd_z_pressed = 0;

int mouse_dx = 0;
int mouse_dy = 0;
int mouse_x = 0;
int mouse_y = 0;
float mouse_wheel_delta = 0.0f;
int mouse_left_down = 0;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
int mouse_middle_pressed = 0;

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
	kbd_F12_pressed = IsKeyPressed(KEY_F12) || ((IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER));

	kbd_y_pressed = IsKeyPressed(KEY_Y);
	kbd_n_pressed = IsKeyPressed(KEY_N);

	mouse_left_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
	mouse_left_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	mouse_right_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
	mouse_middle_pressed = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);

	kbd_energy_bomb_pressed = IsKeyPressed(KEY_TAB);
	kbd_hyperspace_pressed = IsKeyPressed(KEY_H);
	kbd_ctrl_pressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
	kbd_jump_pressed = IsKeyPressed(KEY_J);
	kbd_escape_pressed = IsKeyPressed(KEY_ESCAPE);
	kbd_dock_pressed = IsKeyPressed(KEY_C);
	kbd_d_pressed = IsKeyPressed(KEY_D);
	kbd_origin_pressed = IsKeyPressed(KEY_O);
	kbd_find_pressed = IsKeyPressed(KEY_F);

	kbd_target_missile_pressed = IsKeyPressed(KEY_T) || IsKeyPressed(KEY_V);
	kbd_fire_missile_pressed = IsKeyPressed(KEY_M) || IsKeyPressed(KEY_B);
	kbd_unarm_missile_pressed = IsKeyPressed(KEY_U) || (current_screen != SCR_QUIT && IsKeyPressed(KEY_N));

	kbd_pause_pressed = IsKeyPressed(KEY_P);
	kbd_resume_pressed = IsKeyPressed(KEY_R);

	kbd_enter_pressed = IsKeyPressed(KEY_ENTER);
	kbd_backspace_pressed = IsKeyPressed(KEY_BACKSPACE);
	kbd_space_pressed = IsKeyPressed(KEY_SPACE);

	kbd_q_pressed = IsKeyPressed(KEY_Q);
	kbd_shift_down = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
	kbd_lctrl_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
	kbd_z_pressed = IsKeyPressed(KEY_Z);

	if (control_scheme == 0) /* Classic */
	{
		kbd_up_down = IsKeyDown(KEY_UP) || IsKeyDown(KEY_S);
		kbd_down_down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_X);
		kbd_left_down = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_COMMA);
		kbd_right_down = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_PERIOD);

		kbd_w_down = IsKeyDown(KEY_W);
		kbd_s_down = IsKeyDown(KEY_S);
		kbd_a_down = IsKeyDown(KEY_A);
		kbd_d_down = IsKeyDown(KEY_D);
		kbd_q_down = IsKeyDown(KEY_Q);
		kbd_e_down = IsKeyDown(KEY_E);

		kbd_fire_pressed = IsKeyDown(KEY_A) || mouse_left_down;
		kbd_ecm_pressed = IsKeyPressed(KEY_E);
		kbd_inc_speed_pressed = IsKeyDown(KEY_SPACE);
		kbd_dec_speed_pressed = IsKeyDown(KEY_SLASH);

		kbd_up_pressed = IsKeyPressed(KEY_UP) || IsKeyDown(KEY_S);
		kbd_down_pressed = IsKeyPressed(KEY_DOWN) || IsKeyDown(KEY_X);
		kbd_left_pressed = IsKeyPressed(KEY_LEFT) || IsKeyDown(KEY_COMMA);
		kbd_right_pressed = IsKeyPressed(KEY_RIGHT) || IsKeyDown(KEY_PERIOD);
	}
	else /* Modern */
	{
		kbd_q_down = IsKeyDown(KEY_Q) || IsKeyDown(KEY_KP_7);
		kbd_e_down = IsKeyDown(KEY_E) || IsKeyDown(KEY_KP_9);

		if (mouse_flight_mode == 2) /* Modern Keyboard-Only Mode (Mouse Flight: Off) */
		{
			/* Steering is strictly on the Arrow Keys to prevent WASD collisions */
			kbd_up_down = IsKeyDown(KEY_UP);
			kbd_down_down = IsKeyDown(KEY_DOWN);
			kbd_left_down = IsKeyDown(KEY_LEFT);
			kbd_right_down = IsKeyDown(KEY_RIGHT);

			kbd_w_down = 0;
			kbd_s_down = 0;
			kbd_a_down = 0;
			kbd_d_down = 0;
		}
		else /* Modern Mouse Flight Mode (Direct / Virtual Joystick) */
		{
			kbd_up_down = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
			kbd_down_down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
			kbd_left_down = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
			kbd_right_down = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

			kbd_w_down = IsKeyDown(KEY_W);
			kbd_s_down = IsKeyDown(KEY_S);
			kbd_a_down = IsKeyDown(KEY_A);
			kbd_d_down = IsKeyDown(KEY_D);
		}

		kbd_fire_pressed = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_RIGHT_SHIFT) || mouse_left_down;
		kbd_ecm_pressed = IsKeyPressed(KEY_G) || mouse_middle_pressed;
		kbd_inc_speed_pressed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_PAGE_UP);
		kbd_dec_speed_pressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_Z) || IsKeyDown(KEY_PAGE_DOWN);

		kbd_up_pressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
		kbd_down_pressed = IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S);
		kbd_left_pressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
		kbd_right_pressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);
	}

	Vector2 delta = GetMouseDelta();
	mouse_dx = (int)delta.x;
	mouse_dy = (int)delta.y;

	mouse_wheel_delta = GetMouseWheelMove();

	mouse_left_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
	mouse_left_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	mouse_right_pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
	mouse_middle_pressed = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);

	if (IsWindowFocused())
	{
		if (!IsCursorHidden())
			DisableCursor();
	}
	else
	{
		if (IsCursorHidden())
			EnableCursor();
	}

	float scale = (float)GetScreenWidth() / 512.0f;
	float scale_y = (float)GetScreenHeight() / 512.0f;
	if (scale_y < scale) scale = scale_y;
	if (scale > 0.001f)
	{
		int render_w = (int)(512.0f * scale);
		int render_h = (int)(512.0f * scale);
		int offset_x = (GetScreenWidth() - render_w) / 2;
		int offset_y = (GetScreenHeight() - render_h) / 2;
		Vector2 mpos = GetMousePosition();
		mouse_x = (int)((mpos.x - offset_x) / scale);
		mouse_y = (int)((mpos.y - offset_y) / scale);
	}
	else
	{
		mouse_x = 256;
		mouse_y = 192;
	}
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
