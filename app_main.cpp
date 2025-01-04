#include "PTO2_for_BMS.hpp"
#include "resource.h"
#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"

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

Context g_context = {};

/*
* Set a light ON or OFF on Winwing PTO2.
* This function writes arbitrary data to a HID device. You should NOT use it on something other
* than a PTO2. Who knows what could happen?
*/
bool	set_PTO2_light(hid_device *device, PTO2LightID light, bool set_on)
{
	assert(!(light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS));
	if (light < MASTER_CAUTION && light != GEAR_HANDLE_BRIGHTNESS)
		return false;

	unsigned char max_bright = (light == GEAR_HANDLE_BRIGHTNESS ? 0xff : 0x01);
	unsigned char brightness = (set_on ? max_bright : 0x00);
	/* This is the HID packet that controls the PTO2 lights. You just change the light ID and
	 * the brightness value. Send a 0 or 1 to turn on or off a light. The light's brightness is
	 * controlled by a separate group brightness ID, and has 256 values (see enum comments for details).
	 * The gear handle is an exception, it doesn't have an on/off state, instead it is directly
	 * controlled by a brightness setting with values from 0x00 to 0xff, just like the other
	 * brightness IDs.
	 */
	unsigned char send_buf[] = {
		0x02, 0x05, 0xBF, 0x00, 0x00, 0x03, 0x49, light,
		brightness, 0x00, 0x00, 0x00, 0x00, 0x00 };
	return (hid_write(device, send_buf, sizeof(send_buf)) != -1);
}

/*
* Read Falcon's shared memory to check if associated PTO light should be turned on.
*/
static bool	is_light_on(void *shared_mem, PTO2LightID PTO_light)
{
	auto &light_bind = g_context.PTO2_light_assignment_map[PTO_light];
	if (!light_bind.has_value())
		return false;
	const char *shared_mem_bytes = reinterpret_cast<const char *>(shared_mem);
	auto *light_bits = reinterpret_cast<const unsigned int *>(shared_mem_bytes + light_bind->ID.offset);
	return *light_bits & light_bind->ID.light_bit;
}

void	thread_routine()
{
	HANDLE file_map_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, "FalconSharedMemoryArea");
	if (!file_map_handle)
	{
		printf("OpenFileMapping failed\n");
		return;
	}
	LPVOID bms_shared_mem = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
	const FlightData *flight_data = reinterpret_cast<FlightData *>(bms_shared_mem);
	if (!flight_data)
	{
		printf("MapViewOfFile failed\n");
		UnmapViewOfFile(bms_shared_mem);
		return;
	}

	printf("Ready!\n");
	g_context.thread_running = true;
	while (g_context.thread_running)
	{
		bool success = true;
		success = success && set_PTO2_light(g_context.hid_device, GEAR_HANDLE_BRIGHTNESS, is_light_on(bms_shared_mem, GEAR_HANDLE_BRIGHTNESS));
		for (int i = MASTER_CAUTION; i <= HOOK; ++i)
		{
			PTO2LightID PTO_light = static_cast<PTO2LightID>(i);
			success = success && set_PTO2_light(g_context.hid_device, PTO_light, is_light_on(bms_shared_mem, PTO_light));
		}
		if (!success)
		{	// Failed HID write operation, most likely because device is disconnected: stop the thread.
			g_context.thread_running = false;
			g_context.require_device_reopen = true;
		}
		std::this_thread::sleep_for(THREAD_SLEEP_INTERVAL);
	}

	UnmapViewOfFile(bms_shared_mem);
	CloseHandle(file_map_handle);
	set_window_icon(WINDOW_ICON_ID_RED);
}

/*
* Widget that allows selection of a Falcon light to bind to a PTO2 light. Returns whether user
* has changed their selection or not.
*/
static bool	PTO2_light_assign_widget(const char *light_name, PTO2LightID PTO_light_ID)
{
	bool selection_changed = false;
	auto &PTO2_light_bind = g_context.PTO2_light_assignment_map[PTO_light_ID];

	// === ERASE BUTTON ===
	bool disable_button = !PTO2_light_bind.has_value();
	if (disable_button)
		ImGui::BeginDisabled();
	
	ImGui::PushID(PTO_light_ID);
	if (ColoredButton("Erase", { 172, 0, 0 }))
	{
		PTO2_light_bind.reset();
		selection_changed = true;
	}
	ImGui::PopID();
	
	if (disable_button)
		ImGui::EndDisabled();
	
	ImGui::SameLine();

	// === SELECTION COMBO ===
	const char *preview = PTO2_light_bind ? PTO2_light_bind->display_name.c_str() : "";
	if (ImGui::BeginCombo(light_name, preview, ImGuiComboFlags_PopupAlignLeft))
	{
		for (auto const &light : g_context.falcon_lights)
		{
			bool selected = PTO2_light_bind.has_value() && (PTO2_light_bind->ID == light.ID);

			if (ImGui::Selectable(light.display_name.c_str(), selected))
			{
				PTO2_light_bind = light;
				// Only set to true if selected item is different from the one already selected
				selection_changed = selection_changed || !selected;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return selection_changed;
}

void TextCentered(const char *text)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::Text(text);
}

void    render_main_window(ImGuiIO& io)
{
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::SetNextWindowPos({ 0, 0 });
	constexpr ImGuiWindowFlags    flags = ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::GetStyle().WindowBorderSize = 0;
	// Disable this to avoid highlighting issues with combo search box
	io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::Begin("main", nullptr, flags);

	// No need to use atomic CAS here I think, nobody else would set require_device_reopen to false
	if (g_context.require_device_reopen == true)
	{
		hid_close(g_context.hid_device);
		g_context.hid_device = nullptr;
		g_context.require_device_reopen = false;
	}

	// Opens the first WW PTO2 device that it finds. Don't handle multiple PTO2s connected
	// at the same time: who the hell would do that?
	if (!g_context.hid_device
		&& !(g_context.hid_device = hid_open(PTO2_VENDOR_ID, PTO2_PRODUCT_ID, nullptr)))
	{
		ImGui::Text("No Winwing PTO2 detected!");
		ImGui::End();
		return;
	}

	if (g_context.thread_running)
	{
		if (ColoredButton("Disconnect from BMS", { 172, 0, 0 }, { -1, 50 }))
		{
			g_context.thread_running = false;
			g_context.thread.join();
		}
		set_window_icon(WINDOW_ICON_ID_GREEN);
	}
	else
	{
		if (ColoredButton("Connect to BMS", { 0, 172, 0 }, { -1, 50 }))
		{
			g_context.thread = std::jthread(&thread_routine);
		}
		set_window_icon(WINDOW_ICON_ID_RED);
	}

	// Move ImGui cursor down slightly to add some padding below the button
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

	// Ensure thread safety by disabling editing when thread is running. Who needs mutexes anyway?
	bool disable_editing = g_context.thread_running;
	if (disable_editing)
		ImGui::BeginDisabled();

	int has_changed = 0;
	has_changed |= (int)PTO2_light_assign_widget_v2("Gear handle light", PTO2LightID::GEAR_HANDLE_BRIGHTNESS);
	has_changed |= (int)PTO2_light_assign_widget_v2("Master caution", PTO2LightID::MASTER_CAUTION);
	has_changed |= (int)PTO2_light_assign_widget_v2("HOOK light", PTO2LightID::HOOK);
	
	has_changed |= (int)PTO2_light_assign_widget_v2("NOSE gear light", PTO2LightID::NOSE);
	has_changed |= (int)PTO2_light_assign_widget_v2("LEFT gear light", PTO2LightID::LEFT);
	has_changed |= (int)PTO2_light_assign_widget_v2("RIGHT gear light", PTO2LightID::RIGHT);

	has_changed |= (int)PTO2_light_assign_widget_v2("Flaps HALF light", PTO2LightID::HALF);
	has_changed |= (int)PTO2_light_assign_widget_v2("Flaps FULL light", PTO2LightID::FULL);
	has_changed |= (int)PTO2_light_assign_widget_v2("Yellow FLAPS light", PTO2LightID::FLAPS);
	
	has_changed |= (int)PTO2_light_assign_widget_v2("JETT button", PTO2LightID::JETTISON);
	has_changed |= (int)PTO2_light_assign_widget_v2("CTR station", PTO2LightID::STATION_CTR);
	has_changed |= (int)PTO2_light_assign_widget_v2("LI station", PTO2LightID::STATION_LI);
	has_changed |= (int)PTO2_light_assign_widget_v2("RI station", PTO2LightID::STATION_RI);
	has_changed |= (int)PTO2_light_assign_widget_v2("LO station", PTO2LightID::STATION_LO);
	has_changed |= (int)PTO2_light_assign_widget_v2("RO station", PTO2LightID::STATION_RO);

	if (has_changed)
		serialize_PTO2_mapping_to_conf_file(g_context.PTO2_light_assignment_map);

	if (disable_editing)
		ImGui::EndDisabled();

	TextCentered("This small app is not made to run alongside SimAppPro.");
	TextCentered("It is instead a complete replacement.");

	ImGui::End();
}
