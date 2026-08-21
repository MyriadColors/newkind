/*
 * Elite - The New Kind (Raylib Port)
 *
 * Raylib version of graphics routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "raylib.h"

#include "config.h"
#include "gfx.h"
#include "elite.h"
#include "game_state.h"

#define aspect_ratio_mode (get_config_state()->aspect_ratio_mode)
#define scaling_filter (get_config_state()->scaling_filter)
#define scanner_filename (get_config_state()->scanner_filename)
#define display_mode (get_config_state()->display_mode)

static RenderTexture2D target;
static Texture2D scanner_tex;
static Texture2D blake_tex;
static bool scanner_loaded = false;
static bool blake_loaded = false;
static bool render_target_active = false;
static bool scissor_active = false;

#define MAX_POLYS 100

static int start_poly;
static int total_polys;

struct poly_data
{
	int z;
	int no_points;
	int face_colour;
	int point_list[16];
	int next;
};

static struct poly_data poly_chain[MAX_POLYS];

static Color palette_to_color(int col)
{
	switch (col)
	{
		case GFX_COL_BLACK:       return BLACK;
		case GFX_COL_WHITE:       return WHITE;
		case GFX_COL_WHITE_2:     return LIGHTGRAY;
		case GFX_COL_RED:         return RED;
		case GFX_COL_RED_3:       return MAROON;
		case GFX_COL_RED_4:       return RED;
		case GFX_COL_DARK_RED:    return MAROON;
		case GFX_COL_GOLD:        return GOLD;
		case GFX_COL_CYAN:        return SKYBLUE;
		case GFX_COL_GREY_1:      return LIGHTGRAY;
		case GFX_COL_GREY_2:      return GRAY;
		case GFX_COL_GREY_3:      return DARKGRAY;
		case GFX_COL_GREY_4:      return LIGHTGRAY;
		case GFX_COL_BLUE_1:      return BLUE;
		case GFX_COL_BLUE_2:      return SKYBLUE;
		case GFX_COL_BLUE_3:      return DARKBLUE;
		case GFX_COL_BLUE_4:      return DARKBLUE;
		case GFX_COL_YELLOW_1:
		case GFX_COL_YELLOW_3:
		case GFX_COL_YELLOW_4:
		case GFX_COL_YELLOW_5:    return YELLOW;
		case GFX_ORANGE_1:
		case GFX_ORANGE_2:
		case GFX_ORANGE_3:        return ORANGE;
		case GFX_COL_GREEN_1:     return DARKGREEN;
		case GFX_COL_GREEN_2:     return GREEN;
		case GFX_COL_GREEN_3:     return LIME;
		case GFX_COL_PINK_1:      return PINK;
		default:                  return WHITE;
	}
}

void xor_mode(int mode)
{
	(void)mode;
}

int gfx_graphics_startup(void)
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(800, 600, "Elite - The New Kind (Raylib)");
	SetTargetFPS(60);

	gfx_apply_display_mode(display_mode);

	target = LoadRenderTexture(512, 512);
	SetTextureFilter(target.texture, (scaling_filter == 1) ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_POINT);

	if (FileExists("scanner.bmp"))
	{
		scanner_tex = LoadTexture("scanner.bmp");
		scanner_loaded = true;
	}

	if (FileExists("blake.bmp"))
	{
		blake_tex = LoadTexture("blake.bmp");
		blake_loaded = true;
	}

	BeginTextureMode(target);
	ClearBackground(BLACK);
	render_target_active = true;

	DisableCursor();

	return 0;
}

void gfx_graphics_shutdown(void)
{
	EnableCursor();

	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}

	if (render_target_active)
	{
		EndTextureMode();
		render_target_active = false;
	}

	if (scanner_loaded) UnloadTexture(scanner_tex);
	if (blake_loaded) UnloadTexture(blake_tex);
	UnloadRenderTexture(target);
	CloseWindow();
}

static int vp_offset_x = 0;
static int vp_offset_y = 0;
static int vp_render_w = 512;
static int vp_render_h = 512;

void gfx_get_viewport(int *x, int *y, int *w, int *h)
{
	if (x) *x = vp_offset_x;
	if (y) *y = vp_offset_y;
	if (w) *w = vp_render_w;
	if (h) *h = vp_render_h;
}

void gfx_update_screen(void)
{
	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}

	if (render_target_active)
	{
		EndTextureMode();
		render_target_active = false;
	}

	/* Update texture filtering based on user setting */
	SetTextureFilter(target.texture, (scaling_filter == 1) ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_POINT);

	BeginDrawing();
	ClearBackground(BLACK);

	int screen_w = GetScreenWidth();
	int screen_h = GetScreenHeight();

	if (aspect_ratio_mode == ASPECT_RATIO_STRETCH)
	{
		vp_render_w = screen_w;
		vp_render_h = screen_h;
		vp_offset_x = 0;
		vp_offset_y = 0;
	}
	else if (aspect_ratio_mode == ASPECT_RATIO_INTEGER)
	{
		int scale = (int)fminf((float)screen_w / 512.0f, (float)screen_h / 512.0f);
		if (scale < 1) scale = 1;
		vp_render_w = 512 * scale;
		vp_render_h = 512 * scale;
		vp_offset_x = (screen_w - vp_render_w) / 2;
		vp_offset_y = (screen_h - vp_render_h) / 2;
	}
	else
	{
		float target_aspect = 1.0f;
		if (aspect_ratio_mode == ASPECT_RATIO_4_3)
			target_aspect = 4.0f / 3.0f;
		else if (aspect_ratio_mode == ASPECT_RATIO_16_9)
			target_aspect = 16.0f / 9.0f;
		else /* ASPECT_RATIO_1_1 */
			target_aspect = 1.0f;

		float win_aspect = (float)screen_w / (float)screen_h;
		if (win_aspect > target_aspect)
		{
			/* Pillarbox (bars on left/right) */
			vp_render_h = screen_h;
			vp_render_w = (int)roundf((float)screen_h * target_aspect);
		}
		else
		{
			/* Letterbox (bars on top/bottom) */
			vp_render_w = screen_w;
			vp_render_h = (int)roundf((float)screen_w / target_aspect);
		}

		vp_offset_x = (screen_w - vp_render_w) / 2;
		vp_offset_y = (screen_h - vp_render_h) / 2;
	}

	Rectangle srcRec = { 0.0f, 0.0f, 512.0f, -512.0f };
	Rectangle dstRec = { (float)vp_offset_x, (float)vp_offset_y, (float)vp_render_w, (float)vp_render_h };
	Vector2 origin = { 0.0f, 0.0f };

	DrawTexturePro(target.texture, srcRec, dstRec, origin, 0.0f, WHITE);
	EndDrawing();

	BeginTextureMode(target);
	render_target_active = true;
}

void gfx_acquire_screen(void)
{
}

void gfx_release_screen(void)
{
}

void gfx_plot_pixel(int x, int y, int col)
{
	int px = x + GFX_X_OFFSET;
	int py = y + GFX_Y_OFFSET;
	if (px >= 0 && px < 512 && py >= 0 && py < 512)
		DrawPixel(px, py, palette_to_color(col));
}

void gfx_fast_plot_pixel(int x, int y, int col)
{
	int px = x + GFX_X_OFFSET;
	int py = y + GFX_Y_OFFSET;
	if (px >= 0 && px < 512 && py >= 0 && py < 512)
		DrawPixel(px, py, palette_to_color(col));
}

void gfx_draw_filled_circle(int cx, int cy, int radius, int circle_colour)
{
	DrawCircle(cx + GFX_X_OFFSET, cy + GFX_Y_OFFSET, (float)radius, palette_to_color(circle_colour));
}

void gfx_draw_circle(int cx, int cy, int radius, int circle_colour)
{
	DrawCircleLines(cx + GFX_X_OFFSET, cy + GFX_Y_OFFSET, (float)radius, palette_to_color(circle_colour));
}

void gfx_draw_line(int x1, int y1, int x2, int y2)
{
	DrawLine(x1 + GFX_X_OFFSET, y1 + GFX_Y_OFFSET, x2 + GFX_X_OFFSET, y2 + GFX_Y_OFFSET, WHITE);
}

void gfx_draw_colour_line(int x1, int y1, int x2, int y2, int line_colour)
{
	DrawLine(x1 + GFX_X_OFFSET, y1 + GFX_Y_OFFSET, x2 + GFX_X_OFFSET, y2 + GFX_Y_OFFSET, palette_to_color(line_colour));
}

void gfx_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int col)
{
	Vector2 p1 = { (float)(x1 + GFX_X_OFFSET), (float)(y1 + GFX_Y_OFFSET) };
	Vector2 p2 = { (float)(x2 + GFX_X_OFFSET), (float)(y2 + GFX_Y_OFFSET) };
	Vector2 p3 = { (float)(x3 + GFX_X_OFFSET), (float)(y3 + GFX_Y_OFFSET) };
	Color c = palette_to_color(col);
	DrawTriangle(p1, p2, p3, c);
	DrawTriangle(p1, p3, p2, c);
}

void gfx_draw_rectangle(int tx, int ty, int bx, int by, int col)
{
	int x = tx + GFX_X_OFFSET;
	int y = ty + GFX_Y_OFFSET;
	int w = bx - tx + 1;
	int h = by - ty + 1;
	DrawRectangle(x, y, w, h, palette_to_color(col));
}

void gfx_display_text(int x, int y, const char *txt)
{
	int rx = x + GFX_X_OFFSET;
	int ry = y + GFX_Y_OFFSET;
	DrawText(txt, rx, ry, 12, WHITE);
}

void gfx_display_colour_text(int x, int y, const char *txt, int col)
{
	int rx = x + GFX_X_OFFSET;
	int ry = y + GFX_Y_OFFSET;
	DrawText(txt, rx, ry, 12, palette_to_color(col));
}

void gfx_display_centre_text(int y, const char *str, int psize, int col)
{
	(void)psize;
	int font_size = 12;
	int text_width = MeasureText(str, font_size);
	int rx = (512 - text_width) / 2;
	int ry = y + GFX_Y_OFFSET;
	DrawText(str, rx, ry, font_size, palette_to_color(col));
}

void gfx_clear_display(void)
{
	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}
	DrawRectangle(GFX_X_OFFSET, GFX_Y_OFFSET, 512, 385, BLACK);
}

void gfx_clear_text_area(void)
{
	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}
	DrawRectangle(GFX_X_OFFSET, GFX_Y_OFFSET + 338, 512, 47, BLACK);
}

void gfx_clear_area(int tx, int ty, int bx, int by)
{
	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}
	int x = tx + GFX_X_OFFSET;
	int y = ty + GFX_Y_OFFSET;
	int w = bx - tx + 1;
	int h = by - ty + 1;
	DrawRectangle(x, y, w, h, BLACK);
}

void gfx_display_pretty_text(int tx, int ty, int bx, int by, const char *txt)
{
	(void)by;
	char strbuf[100];
	const char *str = txt;
	int len = strlen(txt);
	int maxlen = (bx - tx) / 8;

	while (len > 0)
	{
		int pos = maxlen;
		if (pos > len) pos = len;

		while (pos > 0 && str[pos] != ' ' && str[pos] != ',' && str[pos] != '.' && str[pos] != '\0')
		{
			pos--;
		}

		if (pos == 0) pos = maxlen;

		strncpy(strbuf, str, pos);
		strbuf[pos] = '\0';

		gfx_display_text(tx, ty, strbuf);

		ty += 12;

		str += pos;
		while (*str == ' ') str++;

		len = strlen(str);
	}
}

void gfx_draw_scanner(void)
{
	if (scanner_loaded)
	{
		DrawTexture(scanner_tex, GFX_X_OFFSET, 385 + GFX_Y_OFFSET, WHITE);
	}
	else
	{
		DrawRectangleLines(GFX_X_OFFSET, 385 + GFX_Y_OFFSET, 512, 127, DARKGRAY);
		DrawText("SCANNER", GFX_X_OFFSET + 220, 385 + GFX_Y_OFFSET + 50, 16, GRAY);
	}
}

void gfx_set_clip_region(int tx, int ty, int bx, int by)
{
	if (scissor_active)
	{
		EndScissorMode();
		scissor_active = false;
	}

	/* Full viewport [0..1, 0..1, 509..511, 381..384] doesn't need scissor clipping */
	if (tx <= 1 && ty <= 1 && bx >= 509 && by >= 381)
	{
		return;
	}

	if (tx >= 0 && ty >= 0 && bx > tx && by > ty)
	{
		int sx = tx + GFX_X_OFFSET;
		int sy = ty + GFX_Y_OFFSET;
		int sw = bx - tx + 1;
		int sh = by - ty + 1;
		if (sx < 0) sx = 0;
		if (sy < 0) sy = 0;
		if (sx + sw > 512) sw = 512 - sx;
		if (sy + sh > 512) sh = 512 - sy;
		if (sw > 0 && sh > 0)
		{
			BeginScissorMode(sx, sy, sw, sh);
			scissor_active = true;
		}
	}
}

void gfx_polygon(int num_points, int *poly_list, int face_colour)
{
	Color col = palette_to_color(face_colour);

	if (num_points < 3)
		return;

	Vector2 p0 = { (float)(poly_list[0] + GFX_X_OFFSET), (float)(poly_list[1] + GFX_Y_OFFSET) };

	for (int i = 1; i < num_points - 1; i++)
	{
		Vector2 p1 = { (float)(poly_list[i*2] + GFX_X_OFFSET), (float)(poly_list[i*2+1] + GFX_Y_OFFSET) };
		Vector2 p2 = { (float)(poly_list[(i+1)*2] + GFX_X_OFFSET), (float)(poly_list[(i+1)*2+1] + GFX_Y_OFFSET) };
		DrawTriangle(p0, p1, p2, col);
		DrawTriangle(p0, p2, p1, col);
	}

	for (int i = 0; i < num_points; i++)
	{
		int next = (i + 1) % num_points;
		DrawLine((int)poly_list[i*2] + GFX_X_OFFSET, (int)poly_list[i*2+1] + GFX_Y_OFFSET,
		         (int)poly_list[next*2] + GFX_X_OFFSET, (int)poly_list[next*2+1] + GFX_Y_OFFSET, col);
	}
}

void gfx_draw_sprite(int sprite_no, int x, int y)
{
	int rx = x + GFX_X_OFFSET;
	int ry = y + GFX_Y_OFFSET;

	switch (sprite_no)
	{
		case IMG_GREEN_DOT:
			DrawCircle(rx + 2, ry + 2, 3, GREEN);
			break;

		case IMG_RED_DOT:
			DrawCircle(rx + 2, ry + 2, 3, RED);
			break;

		case IMG_BIG_S:
			DrawText("S", rx, ry, 16, GREEN);
			break;

		case IMG_ELITE_TXT:
			DrawText("ELITE", rx, ry, 24, GOLD);
			break;

		case IMG_BIG_E:
			DrawText("E", rx, ry, 16, RED);
			break;

		case IMG_BLAKE:
			if (blake_loaded)
				DrawTexture(blake_tex, rx, ry, WHITE);
			else
				DrawRectangle(rx, ry, 32, 32, GRAY);
			break;

		case IMG_MISSILE_GREEN:
			DrawRectangle(rx, ry, 8, 12, GREEN);
			break;

		case IMG_MISSILE_YELLOW:
			DrawRectangle(rx, ry, 8, 12, YELLOW);
			break;

		case IMG_MISSILE_RED:
			DrawRectangle(rx, ry, 8, 12, RED);
			break;

		default:
			break;
	}
}

void gfx_start_render(void)
{
	start_poly = 0;
	total_polys = 0;
}

void gfx_render_polygon(int num_points, int *point_list, int face_colour, int zavg)
{
	if (total_polys == MAX_POLYS)
		return;

	int x = total_polys++;

	poly_chain[x].no_points = num_points;
	poly_chain[x].face_colour = face_colour;
	poly_chain[x].z = zavg;
	poly_chain[x].next = -1;

	for (int i = 0; i < 16; i++)
		poly_chain[x].point_list[i] = point_list[i];

	if (x == 0)
		return;

	if (zavg > poly_chain[start_poly].z)
	{
		poly_chain[x].next = start_poly;
		start_poly = x;
		return;
	}

	int i;
	for (i = start_poly; poly_chain[i].next != -1; i = poly_chain[i].next)
	{
		int nx = poly_chain[i].next;
		if (zavg > poly_chain[nx].z)
		{
			poly_chain[i].next = x;
			poly_chain[x].next = nx;
			return;
		}
	}

	poly_chain[i].next = x;
}

void gfx_render_line(int x1, int y1, int x2, int y2, int dist, int col)
{
	int point_list[4];
	point_list[0] = x1;
	point_list[1] = y1;
	point_list[2] = x2;
	point_list[3] = y2;
	gfx_render_polygon(2, point_list, col, dist);
}

void gfx_finish_render(void)
{
	if (total_polys == 0)
		return;

	for (int i = start_poly; i != -1; i = poly_chain[i].next)
	{
		int num_points = poly_chain[i].no_points;
		int *pl = poly_chain[i].point_list;
		int col = poly_chain[i].face_colour;

		if (num_points == 2)
		{
			gfx_draw_colour_line(pl[0], pl[1], pl[2], pl[3], col);
			continue;
		}

		gfx_polygon(num_points, pl, col);
	}
}

int gfx_request_file(const char *title, char *path, const char *ext)
{
	char input_buf[256];
	strncpy(input_buf, path, sizeof(input_buf) - 1);
	input_buf[sizeof(input_buf) - 1] = '\0';
	int len = strlen(input_buf);

	while (!WindowShouldClose())
	{
		int key = GetCharPressed();
		while (key > 0)
		{
			if ((key >= 32) && (key <= 126) && (len < 60))
			{
				input_buf[len++] = (char)key;
				input_buf[len] = '\0';
			}
			key = GetCharPressed();
		}

		if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
		{
			input_buf[--len] = '\0';
		}

		if (IsKeyPressed(KEY_ENTER))
		{
			if (len > 0)
			{
				if (ext != nullptr && ext[0] != '\0' && !strstr(input_buf, "."))
				{
					strcat(input_buf, ".");
					strcat(input_buf, ext);
				}
				strcpy(path, input_buf);
				return 1;
			}
			return 0;
		}

		if (IsKeyPressed(KEY_ESCAPE))
		{
			return 0;
		}

		/* Draw the modal dialog box */
		gfx_clear_area(64, 120, 448, 260);
		gfx_draw_rectangle(64, 120, 448, 260, GFX_COL_BLACK);
		gfx_draw_colour_line(64, 120, 448, 120, GFX_COL_GOLD);
		gfx_draw_colour_line(64, 260, 448, 260, GFX_COL_GOLD);
		gfx_draw_colour_line(64, 120, 64, 260, GFX_COL_GOLD);
		gfx_draw_colour_line(448, 120, 448, 260, GFX_COL_GOLD);

		gfx_display_centre_text(135, title, 140, GFX_COL_GOLD);
		gfx_display_centre_text(165, "Enter filename (Enter: confirm, Esc: cancel):", 120, GFX_COL_WHITE);

		/* Input text field */
		gfx_draw_rectangle(80, 190, 432, 220, GFX_COL_GREY_3);
		gfx_draw_colour_line(80, 190, 432, 190, GFX_COL_WHITE);
		gfx_draw_colour_line(80, 220, 432, 220, GFX_COL_WHITE);
		gfx_draw_colour_line(80, 190, 80, 220, GFX_COL_WHITE);
		gfx_draw_colour_line(432, 190, 432, 220, GFX_COL_WHITE);

		char display_buf[260];
		snprintf(display_buf, sizeof(display_buf), "%s_", input_buf);
		gfx_display_text(90, 200, display_buf);

		gfx_update_screen();
	}

	return 0;
}

void gfx_toggle_maximize (void)
{
	if (IsWindowFullscreen())
	{
		ToggleFullscreen();
	}

	if (IsWindowMaximized())
		RestoreWindow();
	else
		MaximizeWindow();
}

void gfx_toggle_fullscreen (void)
{
	ToggleFullscreen();
}

int gfx_is_window_maximized (void)
{
	return IsWindowMaximized() ? 1 : 0;
}

int gfx_is_window_fullscreen (void)
{
	return IsWindowFullscreen() ? 1 : 0;
}

void gfx_apply_display_mode (int mode)
{
	int mon = GetCurrentMonitor();
	int mon_w = GetMonitorWidth(mon);
	int mon_h = GetMonitorHeight(mon);

	switch (mode)
	{
		case DISPLAY_MODE_MAXIMIZED:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (!IsWindowMaximized())
				MaximizeWindow();
			break;

		case DISPLAY_MODE_FULLSCREEN:
			if (!IsWindowFullscreen())
				ToggleFullscreen();
			break;

		case DISPLAY_MODE_800_600:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (IsWindowMaximized())
				RestoreWindow();
			SetWindowSize(800, 600);
			SetWindowPosition((mon_w - 800) / 2, (mon_h - 600) / 2);
			break;

		case DISPLAY_MODE_1024_768:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (IsWindowMaximized())
				RestoreWindow();
			SetWindowSize(1024, 768);
			SetWindowPosition((mon_w - 1024) / 2, (mon_h - 768) / 2);
			break;

		case DISPLAY_MODE_1280_720:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (IsWindowMaximized())
				RestoreWindow();
			SetWindowSize(1280, 720);
			SetWindowPosition((mon_w - 1280) / 2, (mon_h - 720) / 2);
			break;

		case DISPLAY_MODE_1920_1080:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (IsWindowMaximized())
				RestoreWindow();
			SetWindowSize(1920, 1080);
			SetWindowPosition((mon_w - 1920) / 2, (mon_h - 1080) / 2);
			break;

		default:
			if (IsWindowFullscreen())
				ToggleFullscreen();
			if (!IsWindowMaximized())
				MaximizeWindow();
			break;
	}
}
