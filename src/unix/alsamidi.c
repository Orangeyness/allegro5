/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      ALSA RawMIDI Sound driver.
 *
 *      By Tom Fjellstrom.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"

#ifdef MIDI_ALSA

#include "allegro/aintern.h"
#include "allegro/aintunix.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/asoundlib.h>

/* external interface to the ALSA rawmidi driver */
static int alsa_rawmidi_detect(int input);
static int alsa_rawmidi_init(int input, int voices);
static void alsa_rawmidi_exit(int input);
static void alsa_rawmidi_output(int data);

static char alsa_rawmidi_desc[80];

static snd_rawmidi_t *rawmidi_handle = NULL;

MIDI_DRIVER midi_alsa =
{
   MIDI_ALSA,						/* id */
   empty_string,					/* name */
   empty_string,					/* desc */
   "ALSA RawMIDI",				/* ASCII name */
   0, 0, 0xFFFF, 0, -1, -1,	/* voices, basevoice, max_voices, def_voices, xmin, xmax */
   alsa_rawmidi_detect,			/* detect */
   alsa_rawmidi_init,			/* init */
   alsa_rawmidi_exit,			/* exit */
   NULL,								/* mixer_volume */
   alsa_rawmidi_output,			/* raw_midi */
   _dummy_load_patches,			/* load_patches */
   _dummy_adjust_patches,		/* adjust_patches */
   _dummy_key_on,					/* key_on */
   _dummy_noop1,					/* key_off */
   _dummy_noop2,					/* set_volume */
   _dummy_noop3,					/* set_pitch */
   _dummy_noop2,					/* set_pan */
   _dummy_noop2					/* set_vibrato */
};

/* alsa_rawmidi_detect:
 *		ALSA RawMIDI detection.
 */
static int alsa_rawmidi_detect(int input)
{
	int card = -1;
	int device = -1;
	int ret = FALSE, err;
	char tmp1[80], tmp2[80], temp[255];
	snd_rawmidi_t *handle = NULL;

	if(input) {
		card = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_input_card", tmp2),
				snd_defaults_rawmidi_card());

		device = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_input_device", tmp2),
				snd_defaults_rawmidi_device());

		if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_INPUT)) < 0) {
			sprintf(temp, "Could not open card/rawmidi device: %s", snd_strerror(err));
			ustrcpy(allegro_error, get_config_text(temp));
			ret = FALSE;
		}

		snd_rawmidi_close(handle);
		
		ret = TRUE;

	}
	else {

		card = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_card", tmp2),
				snd_defaults_rawmidi_card());

		device = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_device", tmp2),
				snd_defaults_rawmidi_device());

		if ((err = snd_rawmidi_open(&handle, card, device, SND_RAWMIDI_OPEN_OUTPUT_APPEND)) < 0) {
			sprintf(temp, "Could not open card/rawmidi device: %s", snd_strerror(err));
			ustrcpy(allegro_error, get_config_text(temp));
			ret = FALSE;
		}
      
		snd_rawmidi_close(handle);

		ret = TRUE;
	}

	return ret;	
}

/* alsa_rawmidi_init:
 *		Setup the ALSA RawMIDI interface.
 */
static int alsa_rawmidi_init(int input, int voices)
{
	int card = -1;
	int device = -1;
	int ret = -1, err;
	char tmp1[80], tmp2[80], temp[255];
	snd_rawmidi_info_t info;

	if(input) {
	
		card = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_input_card", tmp2),
				snd_defaults_rawmidi_card());

		device = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_input_device", tmp2),
				snd_defaults_rawmidi_device());

		if ((err = snd_rawmidi_open(&rawmidi_handle, card, device, SND_RAWMIDI_OPEN_INPUT)) < 0) {
			sprintf(temp, "Could not open card/rawmidi device: %s", snd_strerror(err));
			ustrcpy(allegro_error, get_config_text(temp));
			ret = -1;
		}

		ret = 0;

	}
	else {

		card = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_card", tmp2),
				snd_defaults_rawmidi_card());

		device = get_config_int(uconvert_ascii("sound", tmp1),
				uconvert_ascii("alsa_rawmidi_device", tmp2),
				snd_defaults_rawmidi_device());

		if ((err = snd_rawmidi_open(&rawmidi_handle, card, device, SND_RAWMIDI_OPEN_OUTPUT_APPEND)) < 0) {
			sprintf(temp, "Could not open card/rawmidi device: %s", snd_strerror(err));
			ustrcpy(allegro_error, get_config_text(temp));
			ret = -1;
		}

		ret = 0;
	}

	if(rawmidi_handle) {
		snd_rawmidi_block_mode(rawmidi_handle, 1);
		snd_rawmidi_info(rawmidi_handle, &info);

		strcpy(alsa_rawmidi_desc, info.name);
		midi_alsa.desc = alsa_rawmidi_desc;
		
		LOCK_VARIABLE(alsa_rawmidi_desc);
		LOCK_VARIABLE(rawmidi_handle);
		LOCK_VARIABLE(midi_alsa);
		LOCK_FUNCTION(alsa_rawmidi_output);
	}
	
	return ret;	
}

/* alsa_rawmidi_exit:
 *		Clean up.
 */
static void alsa_rawmidi_exit(int input)
{
	if(rawmidi_handle) {
		snd_rawmidi_output_drain(rawmidi_handle);
		snd_rawmidi_close(rawmidi_handle);
	}
	
	rawmidi_handle = NULL;
}

/* alsa_rawmidi_output:
 *		Outputs MIDI data.
 */
static void alsa_rawmidi_output(int data)
{
	int err;
	
	err = snd_rawmidi_write(rawmidi_handle, &data, sizeof(char));

}
END_OF_STATIC_FUNCTION(alsa_rawmidi_output);

/* alsa_rawmidi_input:
 *		Reads MIDI data.
 */
static INLINE int alsa_rawmidi_input(void)
{
	char data = 0;

	if(snd_rawmidi_read(rawmidi_handle, &data, sizeof(char)) > 0)
		return data;
	else
		return 0;
}

#endif /* MIDI_ALSA */
