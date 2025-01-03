#pragma once

#include <thread>
#include <atomic>

#include <Windows.h>

#include "imgui.h"
#include "hidapi.h"
#include "FlightData.h"
#include "resource.h"

constexpr int	WINDOW_ICON_ID_GREEN	= IDI_ICON1;
constexpr int	WINDOW_ICON_ID_RED		= IDI_ICON2;

constexpr const char		WIN_TITLE[] = "Winwing PTO2 for Falcon BMS";
constexpr int				WIN_WIDTH = 400;
constexpr int				WIN_HEIGHT = 300;
constexpr unsigned short	PTO2_VENDOR_ID = 0x4098;
constexpr unsigned short	PTO2_PRODUCT_ID = 0xbf05;
constexpr auto				THREAD_SLEEP_INTERVAL = std::chrono::milliseconds(100);

extern const char Roboto_Medium_compressed_data_base85[148145 + 1];

ImGuiStyle	get_custom_imgui_style();
void		render_main_window(ImGuiIO& io);

inline void		set_window_icon(int IDI_thing)
{
	// Set Window's icon
	//HWND hWnd = glfwGetWin32Window(window);
	HWND hWnd = GetActiveWindow();
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_thing));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

struct Context
{
	hid_device			*hid_device = nullptr;
	
	std::jthread		thread;
	std::atomic_bool	thread_running = false;
	// Not pretty but I don't really care. Asks for the main thread to close and reopen the HID device.
	// Used in case of hid_write error.
	std::atomic_bool	require_device_reopen = false;
};

extern Context	g_context;