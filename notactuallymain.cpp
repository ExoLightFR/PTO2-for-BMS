#include <cassert>

#include <stdio.h>
#include <Windows.h>

#include "hidapi.h"
#include "FlightData.h"

//  BACKLIGHT FULL:		02 05 BF 00 00 03 49 00 FF 00 00 00 00 00
//  BACKLIGHT OFF:		02 05 BF 00 00 03 49 00 00 00 00 00 00 00
//  GEAR HANDLE BRIGHT:	02 05 BF 00 00 03 49 01 FF 00 00 00 00 00
//  GEAR HANDLE OFF:	02 05 BF 00 00 03 49 01 00 00 00 00 00 00
//  SL BRIGHT:			02 05 BF 00 00 03 49 02 FF 00 00 00 00 00
//  SL DIM:				02 05 BF 00 00 03 49 02 00 00 00 00 00 00
//  FLAG BRIGHT:		02 05 BF 00 00 03 49 03 FF 00 00 00 00 00
//  FLAG BRIGHT:		02 05 BF 00 00 03 49 03 00 00 00 00 00 00
//  MASTER CAUTION ON:	02 05 BF 00 00 03 49 04 01 00 00 00 00 00
//  MASTER CAUTION OFF:	02 05 BF 00 00 03 49 04 00 00 00 00 00 00
//  JETTISON ON:		02 05 BF 00 00 03 49 05 01 00 00 00 00 00
//  JETTISON OFF:		02 05 BF 00 00 03 49 05 00 00 00 00 00 00
//  CTR ON:				02 05 BF 00 00 03 49 06 01 00 00 00 00 00
//  CTR OFF:			02 05 BF 00 00 03 49 06 00 00 00 00 00 00
//  LI ON:				02 05 BF 00 00 03 49 07 01 00 00 00 00 00
//  LI OFF:				02 05 BF 00 00 03 49 07 00 00 00 00 00 00
//  LO ON:				02 05 BF 00 00 03 49 08 01 00 00 00 00 00
//  LO OFF:				02 05 BF 00 00 03 49 08 00 00 00 00 00 00
//  RO ON:				02 05 BF 00 00 03 49 09 01 00 00 00 00 00
//  RO OFF:				02 05 BF 00 00 03 49 09 00 00 00 00 00 00
//  RI ON:				02 05 BF 00 00 03 49 0A 01 00 00 00 00 00
//  RI OFF:				02 05 BF 00 00 03 49 0A 00 00 00 00 00 00
//  FLAPS ON:			02 05 BF 00 00 03 49 0B 01 00 00 00 00 00
//  FLAPS OFF:			02 05 BF 00 00 03 49 0B 00 00 00 00 00 00
//  NOSE ON:			02 05 BF 00 00 03 49 0C 01 00 00 00 00 00
//  NOSE OFF:			02 05 BF 00 00 03 49 0C 00 00 00 00 00 00
//  FULL ON:			02 05 BF 00 00 03 49 0D 01 00 00 00 00 00
//  FULL OFF:			02 05 BF 00 00 03 49 0D 00 00 00 00 00 00
//  RIGHT ON:			02 05 BF 00 00 03 49 0E 01 00 00 00 00 00
//  RIGHT OFF:			02 05 BF 00 00 03 49 0E 00 00 00 00 00 00
//  LEFT ON:			02 05 BF 00 00 03 49 0F 01 00 00 00 00 00
//  LEFT OFF:			02 05 BF 00 00 03 49 0F 00 00 00 00 00 00
//  HALF ON:			02 05 BF 00 00 03 49 10 01 00 00 00 00 00
//  HALF OFF:			02 05 BF 00 00 03 49 10 00 00 00 00 00 00
//  HOOK ON:			02 05 BF 00 00 03 49 11 01 00 00 00 00 00
//  HOOK OFF:			02 05 BF 00 00 03 49 11 00 00 00 00 00 00

enum PTO2_light : unsigned char {
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

bool	set_light(hid_device *device, PTO2_light light, bool set_on)
{
	assert( !(light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS) );
	if (light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS)
		return false;

	unsigned char max_bright = (light == GEAR_HANDLE_BRIGHTNESS ? 0xff : 0x01);
	unsigned char brightness = (set_on ? max_bright : 0x00);
	unsigned char send_buf[] = {
		0x02, 0x05, 0xBF, 0x00,
		0x00, 0x03, 0x49, light,
		brightness, 0x00, 0x00, 0x00,
		0x00, 0x00 };
	return (hid_write(device, send_buf, sizeof(send_buf)) != -1);
}

struct AllLightBits {
	unsigned int light_bits;
	unsigned int light_bits2;
	unsigned int light_bits3;
	bool operator==(AllLightBits const&) const = default;
};

int	main(void)
{
	if (hid_init() != 0)
		return 1;

	hid_device *handle = hid_open(0x4098, 0xbf05, NULL);
	if (!handle) {
		printf("unable to open device\n");
		hid_exit();
		return 1;
	}

	set_light(handle, JETTISON, false);

	HANDLE file_map_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, "FalconSharedMemoryArea");
	if (!file_map_handle)
	{
		printf("Failed to open shared memory thingy\n");
		return 1;
	}
	LPVOID bms_shared_mem = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
	const FlightData* flight_data = reinterpret_cast<FlightData*>(bms_shared_mem);
	if (!flight_data)
	{
		printf("Fuck\n");
		return 1;
	}

	printf("Ready!\n");
	AllLightBits old_lights = {};
	while (true)
	{
		AllLightBits new_lights = { flight_data->lightBits, flight_data->lightBits2, flight_data->lightBits3 };
		if (old_lights != new_lights)
		{
			set_light(handle, MASTER_CAUTION, flight_data->IsSet(FlightData::MasterCaution));
			set_light(handle, GEAR_HANDLE_BRIGHTNESS, flight_data->IsSet2(FlightData::GEARHANDLE));
			set_light(handle, NOSE, flight_data->IsSet3(FlightData::NoseGearDown));
			set_light(handle, LEFT, flight_data->IsSet3(FlightData::LeftGearDown));
			set_light(handle, RIGHT, flight_data->IsSet3(FlightData::RightGearDown));
			set_light(handle, HOOK, flight_data->IsSet(FlightData::Hook));
			old_lights = new_lights;
		}
		Sleep(100);
	}

	UnmapViewOfFile(bms_shared_mem);
	CloseHandle(file_map_handle);

	hid_close(handle);
	hid_exit();
}