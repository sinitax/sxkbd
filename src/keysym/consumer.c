#include "keysym/consumer.h"

#include "hid/consumer.h"

#include <stdint.h>

uint16_t
keysym_to_consumer(uint32_t key)
{
	switch (key) {
		case KS_AUDIO_MUTE:
			return AUDIO_MUTE;
		case KS_AUDIO_VOL_UP:
			return AUDIO_VOL_UP;
		case KS_AUDIO_VOL_DOWN:
			return AUDIO_VOL_DOWN;
		case KS_MEDIA_NEXT_TRACK:
			return TRANSPORT_NEXT_TRACK;
		case KS_MEDIA_PREV_TRACK:
			return TRANSPORT_PREV_TRACK;
		case KS_MEDIA_FAST_FORWARD:
			return TRANSPORT_FAST_FORWARD;
		case KS_MEDIA_REWIND:
			return TRANSPORT_REWIND;
		case KS_MEDIA_STOP:
			return TRANSPORT_STOP;
		case KS_MEDIA_EJECT:
			return TRANSPORT_STOP_EJECT;
		case KS_MEDIA_PLAY_PAUSE:
			return TRANSPORT_PLAY_PAUSE;
		case KS_MEDIA_SELECT:
			return AL_CC_CONFIG;
		case KS_MAIL:
			return AL_EMAIL;
		case KS_CALCULATOR:
			return AL_CALCULATOR;
		case KS_MY_COMPUTER:
			return AL_LOCAL_BROWSER;
		case KS_WWW_SEARCH:
			return AC_SEARCH;
		case KS_WWW_HOME:
			return AC_HOME;
		case KS_WWW_BACK:
			return AC_BACK;
		case KS_WWW_FORWARD:
			return AC_FORWARD;
		case KS_WWW_STOP:
			return AC_STOP;
		case KS_WWW_REFRESH:
			return AC_REFRESH;
		case KS_BRIGHTNESS_UP:
			return BRIGHTNESS_UP;
		case KS_BRIGHTNESS_DOWN:
			return BRIGHTNESS_DOWN;
		case KS_WWW_FAVORITES:
			return AC_BOOKMARKS;
		default:
			return 0;
	}
}
