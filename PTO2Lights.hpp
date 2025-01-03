#pragma once

#include <cstdint>
#include <cassert>
#include "hidapi.h"

class PTO2Lights
{
public:
	enum LightID : unsigned char {
		/* Brightness has 256 values, from 0x00 to 0xff */
		BACKLIGHT_BRIGHTNESS = 0x00,	// Brightness of panel backlight
		GEAR_HANDLE_BRIGHTNESS,			// Gear handle has variable brightness, not on/off
		SL_BRIGHTNESS,					// Buttons brightness (master caution, station select...)
		FLAG_BRIGHTNESS,				// Flags brightness (NOSE/LEFT/RIGHT, HOOK...)
		/* Light have two states, off and on, either 0x00 or 0x01.
		Their brightness is controlled by SL or FLAG_BRIGHTNESS */
		MASTER_CAUTION,
		JETTISON,
		STATION_CTR,
		STATION_LI,
		STATION_LO,
		STATION_RO,
		STATION_RI,
		FLAPS,
		NOSE,
		FULL,
		RIGHT,
		LEFT,
		HALF,
		HOOK
	};

	bool	set_light(hid_device* device, LightID light, bool set_on)
	{
		assert(!(light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS));
		if (light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS)
			return false;

		unsigned char max_bright = (light == GEAR_HANDLE_BRIGHTNESS ? 0xff : 0x01);
		unsigned char brightness = (set_on ? max_bright : 0x00);
		unsigned char send_buf[] = {
			0x02, 0x05, 0xBF, 0x00, 0x00, 0x03, 0x49, light,
			brightness, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		return (hid_write(device, send_buf, sizeof(send_buf)) != -1);
	}

	bool	set_backlight_brightness(uint8_t brightness)
	{
		if (_backlight_brightness == brightness)
			return true;
		unsigned char send_buf[] = {
			0x02, 0x05, 0xBF, 0x00, 0x00, 0x03, 0x49, BACKLIGHT_BRIGHTNESS,
			brightness, 0x00, 0x00, 0x00, 0x00, 0x00
		};
	}

private:
	hid_device	*_device;
	uint8_t	_backlight_brightness;
	uint8_t	_gear_handle_brightness;
	uint8_t	_sl_brightness;
	uint8_t	_flag_brightness;
};