#pragma once

#include <thread>
#include <atomic>

#include <Windows.h>

#include "imgui.h"
#include "hidapi.h"
#include "FlightData.h"

constexpr const char	WIN_TITLE[] = "Winwing PTO2 for Falcon BMS";
constexpr int			WIN_WIDTH = 400;
constexpr int			WIN_HEIGHT = 300;
constexpr auto			THREAD_SLEEP_INTERVAL = std::chrono::milliseconds(100);

extern const char Roboto_Medium_compressed_data_base85[148145 + 1];

ImGuiStyle	get_custom_imgui_style();
void		render_main_window(ImGuiIO& io);

struct Context
{
	hid_device			*hid_device = nullptr;
	HANDLE				falcon_shared_mem_handle = nullptr;
	const FlightData	*flight_data = nullptr;
	
	std::jthread		thread;
	std::atomic_bool	thread_running = false;
	std::atomic_bool	thread_error = false;
};

extern Context	g_context;