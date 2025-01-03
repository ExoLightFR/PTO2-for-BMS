#include "PTO2_for_BMS.hpp"

Context g_context = {};

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
	assert(!(light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS));
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

void    render_main_window(ImGuiIO& io)
{
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::SetNextWindowPos({ 0, 0 });
	constexpr ImGuiWindowFlags    flags = ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoDecoration;
	ImGui::GetStyle().WindowBorderSize = 0;
	ImGui::Begin("main", nullptr, flags);

	if (!g_context.hid_device && !(g_context.hid_device = hid_open(0x4098, 0xbf05, NULL)))
	{
		ImGui::Text("No Winwing PTO2 detected!");
		ImGui::End();
		return;
	}

	ImGui::Text("Hello, world!");

	// HANDLE file_map_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, "FalconSharedMemoryArea");
	// if (!file_map_handle)
	// {
	// 	printf("Failed to open shared memory thingy\n");
	// 	ImGui::Text("Failed to open shared memory thingy\n");
	// 	ImGui::End();
	// 	return;
	// }
	// LPVOID bms_shared_mem = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
	// const FlightData *flight_data = reinterpret_cast<FlightData *>(bms_shared_mem);
	// if (!flight_data)
	// {
	// 	printf("Fuck\n");
	// 	ImGui::Text("Fuck");
	// 	ImGui::End();
	// 	return;
	// }
	// 
	// printf("Ready!\n");
	// //while (true)
	// {
	// 	set_light(g_context.hid_device, MASTER_CAUTION, flight_data->IsSet(FlightData::MasterCaution));
	// 	set_light(g_context.hid_device, GEAR_HANDLE_BRIGHTNESS, flight_data->IsSet2(FlightData::GEARHANDLE));
	// 	set_light(g_context.hid_device, NOSE, flight_data->IsSet3(FlightData::NoseGearDown));
	// 	set_light(g_context.hid_device, LEFT, flight_data->IsSet3(FlightData::LeftGearDown));
	// 	set_light(g_context.hid_device, RIGHT, flight_data->IsSet3(FlightData::RightGearDown));
	// 	set_light(g_context.hid_device, HOOK, flight_data->IsSet(FlightData::Hook));
	// 	//Sleep(100);
	// }
	// 
	// UnmapViewOfFile(bms_shared_mem);
	// CloseHandle(file_map_handle);

	//ImGui::Button("Test", {-1, 0});
	ImGui::End();
}
