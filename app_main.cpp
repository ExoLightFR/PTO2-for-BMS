#include "PTO2_for_BMS.hpp"
#include "resource.h"
#include "nlohmann/json_fwd.hpp"
#include "nlohmann/json.hpp"
#include "IVibeData.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // Will drag system OpenGL headers

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
static bool	set_PTO2_light(hid_device *device, PTO2LightID light, bool set_on)
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
static bool	is_light_on(const void *shared_mem, PTO2LightID PTO_light)
{
	auto &light_bind = g_context.PTO2_light_assignment_map[PTO_light];
	if (!light_bind.has_value())
		return false;
	const char *shared_mem_bytes = reinterpret_cast<const char *>(shared_mem);
	auto *light_bits = reinterpret_cast<const unsigned int *>(shared_mem_bytes + light_bind->ID.offset);
	return *light_bits & light_bind->ID.light_bit;
}

template <class T>
const T*	get_falcon_shared_memory(const char *name)
{
	HANDLE file_map_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, name);
	if (!file_map_handle)
		return nullptr;

	LPVOID shared_mem = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
	if (!shared_mem)
		UnmapViewOfFile(shared_mem);
	
	// This just leaks lmao don't do that
	return reinterpret_cast<const T*>(shared_mem);
}

// TESTME
template <class T>
class FalconSharedMemArea
{
	HANDLE	_file_map_handle = nullptr;
	LPVOID	_shared_mem = nullptr;
public:
	FalconSharedMemArea() = delete;
	FalconSharedMemArea(const char *name) noexcept
	{
		_file_map_handle = OpenFileMappingA(FILE_MAP_READ, FALSE, name);
		if (_file_map_handle)
			_shared_mem = MapViewOfFile(_file_map_handle, FILE_MAP_READ, 0, 0, 0);
	}
	~FalconSharedMemArea() noexcept
	{
		if (_shared_mem)
			UnmapViewOfFile(_shared_mem);
		if (_file_map_handle)
			CloseHandle(_file_map_handle);
	}
	FalconSharedMemArea(FalconSharedMemArea const &) = delete;
	FalconSharedMemArea& operator=(FalconSharedMemArea const &) = delete;

	bool	is_open() const noexcept
	{
		return _file_map_handle && _shared_mem;
	}
	const T *get() const noexcept
	{
		return reinterpret_cast<const T *>(_shared_mem);
	}
	const void *get_raw() const noexcept
	{
		return reinterpret_cast<const void *>(_shared_mem);
	}
};

// TESTME: New routine
void	thread_routine()
{
	FalconSharedMemArea<IntellivibeData> ivibe_data("FalconIntellivibeSharedMemoryArea");
	if (!ivibe_data.is_open())
		return;

	// Connected to BMS, but waiting for sim to be in 3D
	g_context.thread_running = true;
	while (!ivibe_data.get()->In3D)
		std::this_thread::sleep_for(THREAD_SLEEP_INTERVAL);

	FalconSharedMemArea<FlightData>	flight_data("FalconSharedMemoryArea");
	if (!flight_data.is_open())
	{
		g_context.thread_running = false;
		return;
	}
	// Minimize main window to taskbar once we're in 3D and have opened FlightData shared memory
	HWND hWnd = glfwGetWin32Window(g_context.glfw_window);
	(void)SendNotifyMessageA(hWnd, WM_SIZE, SIZE_MINIMIZED, 0);

	while (g_context.thread_running)
	{
		bool success = true;
		success = success && set_PTO2_light(g_context.hid_device, GEAR_HANDLE_BRIGHTNESS,
			is_light_on(flight_data.get_raw(), GEAR_HANDLE_BRIGHTNESS));
		for (int i = MASTER_CAUTION; i <= HOOK; ++i)
		{
			PTO2LightID PTO_light = static_cast<PTO2LightID>(i);
			success = success && set_PTO2_light(g_context.hid_device, PTO_light,
				is_light_on(flight_data.get_raw(), PTO_light));
		}
		if (!success)
		{	// Failed HID write operation, most likely because device is disconnected: stop the thread.
			g_context.thread_running = false;
			g_context.require_device_reopen = true;
		}
		std::this_thread::sleep_for(THREAD_SLEEP_INTERVAL);
	}

	// UnmapViewOfFile(bms_shared_mem);
	// CloseHandle(file_map_handle);
	//set_window_icon(WINDOW_ICON_ID_RED);
}

namespace widgets {
	static void	connect_button()
	{
		float height = ImGui::GetFrameHeight() * 2.0f;

		// Opens the first WW PTO2 device that it finds. Don't handle multiple PTO2s connected
		// at the same time: who the hell would do that?
		if (!g_context.hid_device
			&& !(g_context.hid_device = hid_open(PTO2_VENDOR_ID, PTO2_PRODUCT_ID, nullptr)))
		{
			// No PTO2 device found, don't allow connection to BMS
			ImGui::BeginDisabled();
			ColoredButton("No Winwing PTO2 found", { 127, 127, 127 }, { -1, height });
			ImGui::EndDisabled();
		}
		else
		{
			// PTO2 has been found
			if (g_context.thread_running)
			{
				if (ColoredButton("Disconnect from BMS", { 172, 0, 0 }, { -1, height }))
				{
					g_context.thread_running = false;
					g_context.thread.join();
				}
				set_window_icon(WINDOW_ICON_ID_GREEN);
			}
			else
			{
				if (ColoredButton("Connect to BMS", { 0, 172, 0 }, { -1, height }))
				{
					g_context.thread = std::jthread(&thread_routine);
				}
				set_window_icon(WINDOW_ICON_ID_RED);
			}
		}
		// Add some empty spacing
		ImGui::Spacing();
	}
}


void    render_main_window(ImGuiIO& io)
{
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::SetNextWindowPos({ 0, 0 });
	constexpr ImGuiWindowFlags    flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoMove
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

	widgets::connect_button();

	// Ensure thread safety by disabling editing when thread is running. Who needs mutexes anyway?
	ImGui::BeginDisabled(g_context.thread_running);

	// Don't use a boolean! Doing changed = changed || PTO2_light_assign(...) shorts the rest
	// of the calls to the widget functions, which creates a visual glitch as the combos don't get rendered.
	int has_changed = 0;
	has_changed |= (int)widgets::PTO2_light_assign("Gear handle light", PTO2LightID::GEAR_HANDLE_BRIGHTNESS);
	has_changed |= (int)widgets::PTO2_light_assign("Master caution", PTO2LightID::MASTER_CAUTION);
	has_changed |= (int)widgets::PTO2_light_assign("HOOK light", PTO2LightID::HOOK);
	
	has_changed |= (int)widgets::PTO2_light_assign("NOSE gear light", PTO2LightID::NOSE);
	has_changed |= (int)widgets::PTO2_light_assign("LEFT gear light", PTO2LightID::LEFT);
	has_changed |= (int)widgets::PTO2_light_assign("RIGHT gear light", PTO2LightID::RIGHT);

	has_changed |= (int)widgets::PTO2_light_assign("Flaps HALF light", PTO2LightID::HALF);
	has_changed |= (int)widgets::PTO2_light_assign("Flaps FULL light", PTO2LightID::FULL);
	has_changed |= (int)widgets::PTO2_light_assign("Yellow FLAPS light", PTO2LightID::FLAPS);
	
	has_changed |= (int)widgets::PTO2_light_assign("JETT button", PTO2LightID::JETTISON);
	has_changed |= (int)widgets::PTO2_light_assign("CTR station", PTO2LightID::STATION_CTR);
	has_changed |= (int)widgets::PTO2_light_assign("LI station", PTO2LightID::STATION_LI);
	has_changed |= (int)widgets::PTO2_light_assign("RI station", PTO2LightID::STATION_RI);
	has_changed |= (int)widgets::PTO2_light_assign("LO station", PTO2LightID::STATION_LO);
	has_changed |= (int)widgets::PTO2_light_assign("RO station", PTO2LightID::STATION_RO);

	if (has_changed)
		serialize_PTO2_mapping_to_conf_file(g_context.PTO2_light_assignment_map);

	ImGui::EndDisabled();

	ImGui::End();
}
