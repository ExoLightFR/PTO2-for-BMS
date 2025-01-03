#pragma once

#include <Windows.h>

#include "imgui.h"
#include "hidapi.h"
#include "FlightData.h"

constexpr const char	WIN_TITLE[] = "Winwing PTO2 for Falcon BMS";
constexpr int			WIN_WIDTH = 400;
constexpr int			WIN_HEIGHT = 300;

extern const char Roboto_Medium_compressed_data_base85[148145 + 1];

ImGuiStyle	get_custom_imgui_style();
void    render_main_window(ImGuiIO& io);

struct Context
{
	hid_device			*hid_device = nullptr;
	HANDLE				falcon_shared_mem_handle = nullptr;
	const FlightData	*flight_data = nullptr;
};

extern Context	g_context;