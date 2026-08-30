/*
 * Elite - The New Kind (Raylib Port)
 *
 * Audio Engine, Music Streaming, Embedded MIDI Soft-Synth, Spatial Audio,
 * and Multi-Channel Volume Controls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "sound.h"
#include "game_state.h"

#define NUM_SAMPLES 14
#define NUM_MUSIC   2

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static const char *music_candidates[NUM_MUSIC][6] = {
	{ "theme.ogg", "theme.qoa", "theme.wav", "theme.mp3", "data/theme.ogg", "data/theme.qoa" },
	{ "danube.ogg", "danube.qoa", "danube.wav", "danube.mp3", "data/danube.ogg", "data/danube.qoa" }
};

static const char *midi_files[NUM_MUSIC] = {
	"data/theme.mid",
	"data/danube.mid"
};

/* ========================================================================= */
/* --- Standalone Embedded MIDI Sequencer & Polyphonic Soft-Synth Engine --- */
/* ========================================================================= */

#define MAX_SYNTH_VOICES   32
#define MAX_MIDI_TRACKS    32
#define SYNTH_SAMPLE_RATE  44100
#define NUM_MIDI_CHANNELS  16

typedef struct {
	bool active;
	int note;
	int channel;
	int program;
	float velocity;
	float freq;
	float phase;
	float env_level;
	int env_state;      /* 0: Attack, 1: Decay, 2: Sustain, 3: Release, 4: Off */
	float pan;
	float attack_step;
	float decay_factor;
	float sustain_level;
	float release_factor;
} SynthVoice;

typedef struct {
	const uint8_t *data;
	uint32_t length;
	uint32_t pos;
	uint32_t abs_tick;
	uint8_t running_status;
	bool finished;
} MidiTrackState;

typedef struct {
	bool loaded;
	bool playing;
	bool loop;
	int format;
	int num_tracks;
	int division;             /* Ticks per quarter note (PPQN) */
	uint32_t tempo_us_per_qn; /* Microseconds per quarter note */
	double ticks_per_second;
	double current_tick;
	double tick_accumulator;
	unsigned char *raw_file_data;
	int raw_file_size;
	MidiTrackState tracks[MAX_MIDI_TRACKS];
	uint8_t channel_programs[NUM_MIDI_CHANNELS];
	float channel_volume[NUM_MIDI_CHANNELS];
	float channel_pan[NUM_MIDI_CHANNELS];
} MidiSequencer;

static AudioStream synth_stream = { 0 };
static bool synth_stream_active = false;
static SynthVoice synth_voices[MAX_SYNTH_VOICES];
static MidiSequencer midi_seq = { 0 };
static uint32_t noise_lfsr = 0xACE1u;

/* LFSR Pseudo-random noise for percussion instruments */
static inline float synth_noise(void)
{
	noise_lfsr = (noise_lfsr >> 1) ^ (-(noise_lfsr & 1u) & 0xB400u);
	return ((float)(noise_lfsr & 0xFFFF) / 32768.0f) - 1.0f;
}

/* Helper to read Big-Endian 16-bit integer */
static inline uint16_t read_be16(const uint8_t *p)
{
	return (uint16_t)((p[0] << 8) | p[1]);
}

/* Helper to read Big-Endian 32-bit integer */
static inline uint32_t read_be32(const uint8_t *p)
{
	return (uint32_t)((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

/* Read MIDI Variable Length Quantity */
static uint32_t read_vlq(const uint8_t *data, uint32_t len, uint32_t *pos)
{
	uint32_t value = 0;
	while (*pos < len)
	{
		uint8_t b = data[(*pos)++];
		value = (value << 7) | (b & 0x7F);
		if (!(b & 0x80))
			break;
	}
	return value;
}

static void synth_all_notes_off(void)
{
	for (int i = 0; i < MAX_SYNTH_VOICES; i++)
	{
		synth_voices[i].active = false;
		synth_voices[i].env_state = 4;
		synth_voices[i].env_level = 0.0f;
	}
}

static void synth_note_on(int channel, int note, int velocity, int program)
{
	if (velocity <= 0)
	{
		/* Note off */
		for (int i = 0; i < MAX_SYNTH_VOICES; i++)
		{
			if (synth_voices[i].active && synth_voices[i].channel == channel && synth_voices[i].note == note)
			{
				synth_voices[i].env_state = 3; /* Release */
			}
		}
		return;
	}

	/* Find an existing free voice or oldest releasing voice */
	int best_idx = -1;
	float lowest_env = 999.0f;

	for (int i = 0; i < MAX_SYNTH_VOICES; i++)
	{
		if (!synth_voices[i].active || synth_voices[i].env_state == 4)
		{
			best_idx = i;
			break;
		}
		if (synth_voices[i].env_state == 3 && synth_voices[i].env_level < lowest_env)
		{
			lowest_env = synth_voices[i].env_level;
			best_idx = i;
		}
	}

	if (best_idx < 0)
		best_idx = 0;

	SynthVoice *v = &synth_voices[best_idx];
	v->active = true;
	v->note = note;
	v->channel = channel;
	v->program = program;
	v->velocity = (float)velocity / 127.0f;
	v->freq = 440.0f * powf(2.0f, (float)(note - 69) / 12.0f);
	v->phase = 0.0f;
	v->env_level = 0.0f;
	v->env_state = 0; /* Attack */
	v->pan = midi_seq.channel_pan[channel];

	/* Envelope and timbre parameters tuned by General MIDI family */
	if (channel == 9) /* Percussion channel */
	{
		v->attack_step = 1.0f / (0.002f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.9990f;
		v->sustain_level = 0.0f;
		v->release_factor = 0.995f;
	}
	else if (program >= 40 && program <= 55) /* Strings & Orchestral */
	{
		v->attack_step = 1.0f / (0.045f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.99995f;
		v->sustain_level = 0.75f;
		v->release_factor = 0.9996f;
	}
	else if (program >= 56 && program <= 71) /* Brass & Reeds */
	{
		v->attack_step = 1.0f / (0.025f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.9999f;
		v->sustain_level = 0.70f;
		v->release_factor = 0.9994f;
	}
	else if (program >= 16 && program <= 23) /* Organ */
	{
		v->attack_step = 1.0f / (0.010f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.99999f;
		v->sustain_level = 0.85f;
		v->release_factor = 0.9995f;
	}
	else if (program >= 32 && program <= 39) /* Bass */
	{
		v->attack_step = 1.0f / (0.008f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.9996f;
		v->sustain_level = 0.40f;
		v->release_factor = 0.9990f;
	}
	else /* Piano / Standard default */
	{
		v->attack_step = 1.0f / (0.005f * (float)SYNTH_SAMPLE_RATE);
		v->decay_factor = 0.99985f;
		v->sustain_level = 0.35f;
		v->release_factor = 0.9992f;
	}
}

static void synth_note_off(int channel, int note)
{
	for (int i = 0; i < MAX_SYNTH_VOICES; i++)
	{
		if (synth_voices[i].active && synth_voices[i].channel == channel && synth_voices[i].note == note)
		{
			synth_voices[i].env_state = 3; /* Release */
		}
	}
}

/* Parse next events in all tracks up to current_tick */
static void midi_seq_process_events(void)
{
	if (!midi_seq.loaded || !midi_seq.playing)
		return;

	bool any_track_active = false;

	for (int t = 0; t < midi_seq.num_tracks; t++)
	{
		MidiTrackState *trk = &midi_seq.tracks[t];
		if (trk->finished)
			continue;

		any_track_active = true;

		while (trk->pos < trk->length && (double)trk->abs_tick <= midi_seq.current_tick)
		{
			uint8_t status = trk->data[trk->pos];
			if (status & 0x80)
			{
				trk->running_status = status;
				trk->pos++;
			}
			else
			{
				status = trk->running_status;
			}

			uint8_t msg_type = status & 0xF0;
			uint8_t ch = status & 0x0F;

			if (msg_type == 0x80) /* Note Off */
			{
				uint8_t note = trk->data[trk->pos++];
				trk->pos++; /* Skip velocity */
				synth_note_off(ch, note);
			}
			else if (msg_type == 0x90) /* Note On */
			{
				uint8_t note = trk->data[trk->pos++];
				uint8_t vel = trk->data[trk->pos++];
				synth_note_on(ch, note, vel, midi_seq.channel_programs[ch]);
			}
			else if (msg_type == 0xA0) /* Polyphonic Aftertouch */
			{
				trk->pos += 2;
			}
			else if (msg_type == 0xB0) /* Control Change */
			{
				uint8_t cc = trk->data[trk->pos++];
				uint8_t val = trk->data[trk->pos++];
				if (cc == 7) /* Channel Volume */
					midi_seq.channel_volume[ch] = (float)val / 127.0f;
				else if (cc == 10) /* Pan */
					midi_seq.channel_pan[ch] = (float)val / 127.0f;
				else if (cc == 120 || cc == 123) /* All Sound/Notes Off */
					synth_all_notes_off();
			}
			else if (msg_type == 0xC0) /* Program Change */
			{
				uint8_t prog = trk->data[trk->pos++];
				midi_seq.channel_programs[ch] = prog;
			}
			else if (msg_type == 0xD0) /* Channel Pressure */
			{
				trk->pos += 1;
			}
			else if (msg_type == 0xE0) /* Pitch Bend */
			{
				trk->pos += 2;
			}
			else if (status == 0xFF) /* Meta Event */
			{
				uint8_t meta_type = trk->data[trk->pos++];
				uint32_t meta_len = read_vlq(trk->data, trk->length, &trk->pos);
				if (meta_type == 0x51 && meta_len == 3) /* Set Tempo */
				{
					uint32_t us = (trk->data[trk->pos] << 16) | (trk->data[trk->pos + 1] << 8) | trk->data[trk->pos + 2];
					if (us > 0)
					{
						midi_seq.tempo_us_per_qn = us;
						midi_seq.ticks_per_second = (1000000.0 / (double)midi_seq.tempo_us_per_qn) * (double)midi_seq.division;
					}
				}
				else if (meta_type == 0x2F) /* End of Track */
				{
					trk->finished = true;
				}
				trk->pos += meta_len;
			}
			else if (status == 0xF0 || status == 0xF7) /* SysEx */
			{
				uint32_t sysex_len = read_vlq(trk->data, trk->length, &trk->pos);
				trk->pos += sysex_len;
			}

			/* Read next delta time */
			if (trk->pos < trk->length && !trk->finished)
			{
				uint32_t delta = read_vlq(trk->data, trk->length, &trk->pos);
				trk->abs_tick += delta;
			}
			else
			{
				trk->finished = true;
			}
		}
	}

	/* Check loop condition */
	if (!any_track_active)
	{
		if (midi_seq.loop)
		{
			/* Rewind all tracks */
			midi_seq.current_tick = 0.0;
			midi_seq.tick_accumulator = 0.0;
			for (int t = 0; t < midi_seq.num_tracks; t++)
			{
				MidiTrackState *trk = &midi_seq.tracks[t];
				trk->pos = 0;
				trk->running_status = 0;
				trk->finished = false;
				if (trk->pos < trk->length)
				{
					trk->abs_tick = read_vlq(trk->data, trk->length, &trk->pos);
				}
			}
		}
		else
		{
			midi_seq.playing = false;
		}
	}
}

/* Audio callback invoked on Raylib audio thread to generate synth PCM samples */
static void midi_synth_audio_callback(void *bufferData, unsigned int frames)
{
	int16_t *out = (int16_t *)bufferData;
	float music_vol = (float)get_config_state()->music_volume / 100.0f;

	if (!midi_seq.playing || music_vol <= 0.001f)
	{
		memset(bufferData, 0, frames * 2 * sizeof(int16_t));
		return;
	}

	double seconds_per_frame = 1.0 / (double)SYNTH_SAMPLE_RATE;

	for (unsigned int f = 0; f < frames; f++)
	{
		/* Advance sequencer time */
		midi_seq.tick_accumulator += seconds_per_frame * midi_seq.ticks_per_second;
		while (midi_seq.tick_accumulator >= 1.0)
		{
			midi_seq.current_tick += 1.0;
			midi_seq.tick_accumulator -= 1.0;
			midi_seq_process_events();
		}

		float left_sample = 0.0f;
		float right_sample = 0.0f;

		for (int v = 0; v < MAX_SYNTH_VOICES; v++)
		{
			SynthVoice *sv = &synth_voices[v];
			if (!sv->active)
				continue;

			/* Progress ADSR envelope */
			switch (sv->env_state)
			{
				case 0: /* Attack */
					sv->env_level += sv->attack_step;
					if (sv->env_level >= 1.0f)
					{
						sv->env_level = 1.0f;
						sv->env_state = 1; /* Decay */
					}
					break;
				case 1: /* Decay */
					sv->env_level *= sv->decay_factor;
					if (sv->env_level <= sv->sustain_level)
					{
						sv->env_level = sv->sustain_level;
						sv->env_state = 2; /* Sustain */
					}
					break;
				case 2: /* Sustain */
					break;
				case 3: /* Release */
					sv->env_level *= sv->release_factor;
					if (sv->env_level <= 0.001f)
					{
						sv->env_level = 0.0f;
						sv->env_state = 4;
						sv->active = false;
					}
					break;
				default:
					sv->active = false;
					break;
			}

			if (!sv->active || sv->env_level <= 0.0f)
				continue;

			/* Generate waveform based on instrument type */
			float sample_val = 0.0f;
			sv->phase += (2.0f * (float)M_PI * sv->freq) / (float)SYNTH_SAMPLE_RATE;
			if (sv->phase >= 2.0f * (float)M_PI)
				sv->phase -= 2.0f * (float)M_PI;

			if (sv->channel == 9) /* Drum / Percussion */
			{
				float tone = sinf(sv->phase);
				float noise = synth_noise();
				sample_val = (tone * 0.4f + noise * 0.6f);
			}
			else
			{
				/* Multi-harmonic bandlimited synthesis */
				float s1 = sinf(sv->phase);
				float s2 = sinf(sv->phase * 2.0f);
				float s3 = sinf(sv->phase * 3.0f);
				float square = (sv->phase < (float)M_PI) ? 1.0f : -1.0f;

				if (sv->program >= 40 && sv->program <= 55) /* Strings */
					sample_val = 0.50f * s1 + 0.30f * s2 + 0.20f * s3;
				else if (sv->program >= 56 && sv->program <= 71) /* Brass */
					sample_val = 0.45f * s1 + 0.35f * s2 + 0.20f * square;
				else if (sv->program >= 16 && sv->program <= 23) /* Organ */
					sample_val = 0.40f * s1 + 0.40f * s3 + 0.20f * square;
				else if (sv->program >= 32 && sv->program <= 39) /* Bass */
					sample_val = 0.70f * s1 + 0.30f * s2;
				else /* Piano / Lead */
					sample_val = 0.60f * s1 + 0.25f * s2 + 0.15f * s3;
			}

			float gain = sample_val * sv->env_level * sv->velocity * midi_seq.channel_volume[sv->channel] * 0.25f;
			float pan = sv->pan;
			left_sample += gain * (1.0f - pan);
			right_sample += gain * pan;
		}

		left_sample *= music_vol;
		right_sample *= music_vol;

		/* Clamp to 16-bit PCM range */
		if (left_sample > 1.0f) left_sample = 1.0f;
		if (left_sample < -1.0f) left_sample = -1.0f;
		if (right_sample > 1.0f) right_sample = 1.0f;
		if (right_sample < -1.0f) right_sample = -1.0f;

		out[f * 2] = (int16_t)(left_sample * 32767.0f);
		out[f * 2 + 1] = (int16_t)(right_sample * 32767.0f);
	}
}

static bool load_midi_file(const char *filename)
{
	synth_all_notes_off();

	if (midi_seq.raw_file_data != nullptr)
	{
		UnloadFileData(midi_seq.raw_file_data);
		midi_seq.raw_file_data = nullptr;
		midi_seq.raw_file_size = 0;
	}

	int data_size = 0;
	unsigned char *data = LoadFileData(filename, &data_size);
	if (data == nullptr || data_size < 14)
	{
		midi_seq.loaded = false;
		return false;
	}

	midi_seq.raw_file_data = data;
	midi_seq.raw_file_size = data_size;

	if (memcmp(data, "MThd", 4) != 0)
	{
		UnloadFileData(data);
		midi_seq.raw_file_data = nullptr;
		midi_seq.loaded = false;
		return false;
	}

	midi_seq.format = read_be16(data + 8);
	midi_seq.num_tracks = read_be16(data + 10);
	midi_seq.division = read_be16(data + 12);
	if (midi_seq.division <= 0)
		midi_seq.division = 96;

	if (midi_seq.num_tracks > MAX_MIDI_TRACKS)
		midi_seq.num_tracks = MAX_MIDI_TRACKS;

	midi_seq.tempo_us_per_qn = 500000; /* 120 BPM default */
	midi_seq.ticks_per_second = (1000000.0 / (double)midi_seq.tempo_us_per_qn) * (double)midi_seq.division;
	midi_seq.current_tick = 0.0;
	midi_seq.tick_accumulator = 0.0;

	for (int c = 0; c < NUM_MIDI_CHANNELS; c++)
	{
		midi_seq.channel_programs[c] = 0;
		midi_seq.channel_volume[c] = 0.90f;
		midi_seq.channel_pan[c] = 0.50f;
	}

	uint32_t offset = 14;
	int valid_tracks = 0;

	for (int t = 0; t < midi_seq.num_tracks && offset + 8 <= (uint32_t)data_size; t++)
	{
		if (memcmp(data + offset, "MTrk", 4) != 0)
			break;

		uint32_t track_len = read_be32(data + offset + 4);
		offset += 8;

		if (offset + track_len > (uint32_t)data_size)
			track_len = (uint32_t)data_size - offset;

		MidiTrackState *trk = &midi_seq.tracks[valid_tracks];
		trk->data = data + offset;
		trk->length = track_len;
		trk->pos = 0;
		trk->running_status = 0;
		trk->finished = false;
		trk->abs_tick = read_vlq(trk->data, trk->length, &trk->pos);

		offset += track_len;
		valid_tracks++;
	}

	midi_seq.num_tracks = valid_tracks;
	midi_seq.loaded = (valid_tracks > 0);
	return midi_seq.loaded;
}

/* ========================================================================= */
/* --- Public Audio Engine API & Multi-Channel Mixer --- */
/* ========================================================================= */

void snd_sound_startup (void)
{
	InitAudioDevice();
	if (!IsAudioDeviceReady())
	{
		sound_active = false;
		return;
	}

	sound_active = true;

	/* Load SFX samples */
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

	/* Load streaming audio tracks (OGG / QOA / MP3 / WAV) */
	for (int m = 0; m < NUM_MUSIC; m++)
	{
		music_loaded[m] = false;
		for (int c = 0; c < 6; c++)
		{
			if (FileExists(music_candidates[m][c]))
			{
				music_list[m] = LoadMusicStream(music_candidates[m][c]);
				music_loaded[m] = true;
				break;
			}
		}
	}

	/* Initialize Soft-Synth AudioStream */
	synth_stream = LoadAudioStream(SYNTH_SAMPLE_RATE, 16, 2);
	if (synth_stream.buffer != nullptr)
	{
		SetAudioStreamCallback(synth_stream, midi_synth_audio_callback);
		synth_stream_active = true;
	}

	snd_apply_volumes();
}

void snd_sound_shutdown (void)
{
	if (!sound_active)
		return;

	snd_stop_midi();

	if (synth_stream_active)
	{
		UnloadAudioStream(synth_stream);
		synth_stream = (AudioStream){ 0 };
		synth_stream_active = false;
	}

	if (midi_seq.raw_file_data != nullptr)
	{
		UnloadFileData(midi_seq.raw_file_data);
		midi_seq.raw_file_data = nullptr;
		midi_seq.loaded = false;
	}

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

void snd_set_master_volume (int vol_percent)
{
	if (vol_percent < 0) vol_percent = 0;
	if (vol_percent > 100) vol_percent = 100;
	get_config_state()->master_volume = vol_percent;
	if (sound_active)
	{
		SetMasterVolume((float)vol_percent / 100.0f);
	}
}

void snd_set_music_volume (int vol_percent)
{
	if (vol_percent < 0) vol_percent = 0;
	if (vol_percent > 100) vol_percent = 100;
	get_config_state()->music_volume = vol_percent;
	if (sound_active)
	{
		float vol = (float)vol_percent / 100.0f;
		for (int m = 0; m < NUM_MUSIC; m++)
		{
			if (music_loaded[m])
				SetMusicVolume(music_list[m], vol);
		}
	}
}

void snd_set_sfx_volume (int vol_percent)
{
	if (vol_percent < 0) vol_percent = 0;
	if (vol_percent > 100) vol_percent = 100;
	get_config_state()->sfx_volume = vol_percent;
}

void snd_apply_volumes (void)
{
	const ConfigState *cfg = get_config_state();
	snd_set_master_volume(cfg->master_volume);
	snd_set_music_volume(cfg->music_volume);
	snd_set_sfx_volume(cfg->sfx_volume);
}

void snd_play_sample (int sample_no)
{
	snd_play_sample_pan(sample_no, 0.5f, 1.0f);
}

void snd_play_sample_pan (int sample_no, float pan, float volume)
{
	if (!sound_active)
		return;

	if (sample_no >= 0 && sample_no < NUM_SAMPLES)
	{
		if (sample_list[sample_no].stream.buffer != nullptr)
		{
			if (pan < 0.0f) pan = 0.0f;
			if (pan > 1.0f) pan = 1.0f;
			if (volume < 0.0f) volume = 0.0f;
			if (volume > 1.0f) volume = 1.0f;

			float sfx_master = (float)get_config_state()->sfx_volume / 100.0f;
			float final_vol = volume * sfx_master;

			SetSoundPan(sample_list[sample_no], pan);
			SetSoundVolume(sample_list[sample_no], final_vol);
			PlaySound(sample_list[sample_no]);
		}
	}
}

void snd_play_sample_spatial (int sample_no, double x, double z)
{
	const double max_range = 8192.0;
	float pan = (float)(0.5 + (x / (2.0 * max_range)));
	if (pan < 0.05f) pan = 0.05f;
	if (pan > 0.95f) pan = 0.95f;

	double dist = sqrt(x * x + z * z);
	float atten = (float)(1.0 - (dist / 16384.0));
	if (atten < 0.25f) atten = 0.25f;
	if (atten > 1.0f) atten = 1.0f;

	snd_play_sample_pan(sample_no, pan, atten);
}

void snd_play_midi (int midi_no, int repeat)
{
	if (!sound_active)
		return;

	if (midi_no >= 0 && midi_no < NUM_MUSIC)
	{
		snd_stop_midi();
		current_music_idx = midi_no;

		/* 1. Primary: Stream OGG/QOA/MP3/WAV file if present */
		if (music_loaded[midi_no])
		{
			music_list[midi_no].looping = (repeat != 0);
			SetMusicVolume(music_list[midi_no], (float)get_config_state()->music_volume / 100.0f);
			PlayMusicStream(music_list[midi_no]);
			return;
		}

		/* 2. Secondary: Embedded Soft-Synth fallback for bundled .mid files */
		if (synth_stream_active && FileExists(midi_files[midi_no]))
		{
			if (load_midi_file(midi_files[midi_no]))
			{
				midi_seq.loop = (repeat != 0);
				midi_seq.playing = true;
				PlayAudioStream(synth_stream);
			}
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
	}

	if (synth_stream_active && midi_seq.playing)
	{
		midi_seq.playing = false;
		synth_all_notes_off();
		StopAudioStream(synth_stream);
	}

	current_music_idx = -1;
}

/* ========================================================================= */
/* --- Immersion Sound Cues --- */
/* ========================================================================= */

void snd_trigger_docking_clearance (void)
{
	snd_play_sample_pan(SND_BEEP, 0.5f, 1.0f);
}

void snd_trigger_low_fuel_warning (void)
{
	snd_play_sample_pan(SND_BOOP, 0.5f, 0.8f);
}

void snd_trigger_missile_lock (bool is_locked)
{
	if (is_locked)
		snd_play_sample_pan(SND_BEEP, 0.5f, 0.9f);
	else
		snd_play_sample_pan(SND_BOOP, 0.5f, 0.6f);
}

void snd_trigger_cargo_scoop (void)
{
	snd_play_sample_pan(SND_BEEP, 0.5f, 0.85f);
}
