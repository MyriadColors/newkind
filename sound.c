/*
 * Elite - The New Kind (Raylib Port)
 *
 * Raylib version of sound effects and music handler.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "raylib.h"
#include "sound.h"

#define NUM_SAMPLES 14
#define NUM_MUSIC   2

static Sound sample_list[NUM_SAMPLES];
static Music music_list[NUM_MUSIC];
static bool music_loaded[NUM_MUSIC] = { false, false };
static int current_music_idx = -1;
static bool sound_active = false;

static const char *sample_files[NUM_SAMPLES] = {
	"launch.wav",    /* SND_LAUNCH */
	"crash.wav",     /* SND_CRASH */
	"dock.wav",      /* SND_DOCK */
	"gameover.wav",  /* SND_GAMEOVER */
	"pulse.wav",     /* SND_PULSE */
	"hitem.wav",     /* SND_HIT_ENEMY */
	"explode.wav",   /* SND_EXPLODE */
	"ecm.wav",       /* SND_ECM */
	"missile.wav",   /* SND_MISSILE */
	"hyper.wav",     /* SND_HYPERSPACE */
	"incom1.wav",    /* SND_INCOMMING_FIRE_1 */
	"incom2.wav",    /* SND_INCOMMING_FIRE_2 */
	"beep.wav",      /* SND_BEEP */
	"boop.wav"       /* SND_BOOP */
};

static const char *music_candidates[NUM_MUSIC][4] = {
	{ "theme.ogg", "theme.wav", "theme.mp3", "data/theme.ogg" },     /* SND_ELITE_THEME */
	{ "danube.ogg", "danube.wav", "danube.mp3", "data/danube.ogg" }   /* SND_BLUE_DANUBE */
};

void snd_sound_startup (void)
{
	InitAudioDevice();
	if (!IsAudioDeviceReady())
	{
		sound_active = false;
		return;
	}

	sound_active = true;

	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		if (FileExists(sample_files[i]))
		{
			sample_list[i] = LoadSound(sample_files[i]);
		}
		else
		{
			sample_list[i] = (Sound){ 0 };
		}
	}

	for (int m = 0; m < NUM_MUSIC; m++)
	{
		music_loaded[m] = false;
		for (int c = 0; c < 4; c++)
		{
			if (FileExists(music_candidates[m][c]))
			{
				music_list[m] = LoadMusicStream(music_candidates[m][c]);
				music_loaded[m] = true;
				break;
			}
		}
	}
}

void snd_sound_shutdown (void)
{
	if (!sound_active)
		return;

	snd_stop_midi();

	for (int m = 0; m < NUM_MUSIC; m++)
	{
		if (music_loaded[m])
		{
			UnloadMusicStream(music_list[m]);
			music_loaded[m] = false;
		}
	}

	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		if (sample_list[i].stream.buffer != nullptr)
		{
			UnloadSound(sample_list[i]);
		}
	}

	CloseAudioDevice();
	sound_active = false;
}

void snd_play_sample (int sample_no)
{
	if (!sound_active)
		return;

	if (sample_no >= 0 && sample_no < NUM_SAMPLES)
	{
		if (sample_list[sample_no].stream.buffer != nullptr)
		{
			PlaySound(sample_list[sample_no]);
		}
	}
}

void snd_play_midi (int midi_no, int repeat)
{
	if (!sound_active)
		return;

	if (midi_no >= 0 && midi_no < NUM_MUSIC)
	{
		snd_stop_midi();

		if (music_loaded[midi_no])
		{
			current_music_idx = midi_no;
			music_list[midi_no].looping = (repeat != 0);
			PlayMusicStream(music_list[midi_no]);
		}
	}
}

void snd_update_sound (void)
{
	if (!sound_active)
		return;

	if (current_music_idx >= 0 && current_music_idx < NUM_MUSIC)
	{
		if (music_loaded[current_music_idx] && IsMusicStreamPlaying(music_list[current_music_idx]))
		{
			UpdateMusicStream(music_list[current_music_idx]);
		}
	}
}

void snd_stop_midi (void)
{
	if (!sound_active)
		return;

	if (current_music_idx >= 0 && current_music_idx < NUM_MUSIC)
	{
		if (music_loaded[current_music_idx])
		{
			StopMusicStream(music_list[current_music_idx]);
		}
		current_music_idx = -1;
	}
}
