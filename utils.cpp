#include <string>
#include <stdexcept>
#include <Windows.h>
#include <shellapi.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "PTO2_for_BMS.hpp"
#include "Uxtheme.h"

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // Will drag system OpenGL headers

ImGuiStyle	get_custom_imgui_style(float scale_factor)
{
	ImGuiStyle	style = ImGuiStyle();

	style.FrameRounding = 3;
	// style.ItemSpacing.y = 6;
	style.GrabRounding = 4;
	style.PopupRounding = 3;
	//style.WindowRounding = 4;

	style.ScaleAllSizes(1.5f);
	ImGui::StyleColorsDark(&style);

	style.ScaleAllSizes(scale_factor);

	return style;
}

ImGuiStyle	get_retro_imgui_style(float scale_factor)
{
	ImGuiStyle style = ImGuiStyle();

	//style.ScaleAllSizes(1.5f);
	style.ScaleAllSizes(scale_factor);

	style.FrameBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 4.0f);
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.ScrollbarSize = 16.0f;
	style.ScrollbarRounding = 0.0f;

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text]					= ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]			= ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg]				= ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_ChildBg]				= ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_PopupBg]				= ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
	colors[ImGuiCol_Border]					= ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
	colors[ImGuiCol_BorderShadow]			= ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]				= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]			= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_FrameBgActive]			= ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TitleBg]				= ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	colors[ImGuiCol_TitleBgActive]			= ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]		= ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]				= ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]			= ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabHovered]	= ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive]	= ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark]				= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab]				= ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	colors[ImGuiCol_SliderGrabActive]		= ImVec4(0.46f, 0.54f, 0.80f, 0.60f);

	// colors[ImGuiCol_Button]					= ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	// colors[ImGuiCol_ButtonHovered]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	// colors[ImGuiCol_ButtonActive]			= ImVec4(0.06f, 0.53f, 0.98f, 1.00f);

	// Grey buttons instead of blue
	colors[ImGuiCol_Button]					= ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_ButtonHovered]			= ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_ButtonActive]			= ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

	colors[ImGuiCol_Header]					= ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered]			= ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive]			= ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator]				= ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
	colors[ImGuiCol_SeparatorHovered]		= ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive]		= ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip]				= ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
	colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]		= ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab]					= ImVec4(0.76f, 0.80f, 0.84f, 0.95f);
	colors[ImGuiCol_TabHovered]				= ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_TabActive]				= ImVec4(0.60f, 0.73f, 0.88f, 0.95f);
	colors[ImGuiCol_TabUnfocused]			= ImVec4(0.92f, 0.92f, 0.94f, 0.95f);
	colors[ImGuiCol_TabUnfocusedActive]		= ImVec4(0.74f, 0.82f, 0.91f, 1.00f);
	colors[ImGuiCol_PlotLines]				= ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]		= ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]			= ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]	= ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget]			= ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_NavHighlight]			= colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]		= ImVec4(0.20f, 0.20f, 0.20f, 0.65f);

	return style;
}

/*
* Change ImGui's style and our custom font's size to match given DPI.
* Standard DPI (100% scale) is considered to be 96.
*/
void	set_app_style(UINT new_dpi, bool retro_mode)
{
	// Set ImGui style scale based on the screen's scaling factor (DPIs)
	float scale_factor = (float)new_dpi / USER_DEFAULT_SCREEN_DPI;
	if (retro_mode)
		ImGui::GetStyle() = get_retro_imgui_style(scale_factor);
	else
		ImGui::GetStyle() = get_custom_imgui_style(scale_factor);

	// Reload our custom font, taking into account the new scale.
	// https://github.com/ocornut/imgui/issues/6547
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();
	const char *font = retro_mode
		? W95_font_compressed_data_base85
		: Roboto_Medium_compressed_data_base85;
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(font, std::trunc(17 * scale_factor));
	io.Fonts->Build();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_CreateDeviceObjects();

	// Set Win32 window theme to either native or Windows 95 style for retro mode
	HWND hWnd = glfwGetWin32Window(g_context.glfw_window);
	LPCWSTR window_theme = (g_context.retro_mode ? L"" : nullptr);
	SetWindowTheme(hWnd, window_theme, window_theme);
}

namespace widgets {

	/*
	* ImGui button with custom color. Calculates appropriate Hovered and Active colors.
	*/
	bool	ColoredButton(const char* label, ImColor color, const ImVec2& size)
	{
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(color.Value.x, color.Value.y, color.Value.z, h, s, v);

		ImGui::PushStyleColor(ImGuiCol_Button,			(ImVec4)ImColor::HSV(h, s, v));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	(ImVec4)ImColor::HSV(h, s, v + 0.1f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	(ImVec4)ImColor::HSV(h, s, v + 0.2f));
		bool pressed = ImGui::Button(label, size);
		ImGui::PopStyleColor(3);

		return pressed;
	}

	/*
	* ImGui text that's centered across the ImGui window.
	*/
	void	TextCentered(const char* text)
	{
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(text).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text);
	}

} // namespace widgets

/*
* Set the window's icon with the Win32 API. Icon has to be a resource thingy in Visual Studio.
* Don't use GLFW: for once, it's easier to use the Win32 API...
*/
void		set_window_icon(int IDI_thing)
{
	// Set Window's icon
	//HWND hWnd = glfwGetWin32Window(window);
	HWND hWnd = GetActiveWindow();
	HINSTANCE hInstance = GetModuleHandleA(NULL);
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_thing));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

/*
* Add the app's system tray icon. Only call when no icon is present! Otherwise, call
* change_tray_icon().
*/
void	add_tray_icon(int IDI_thingy)
{
	HMODULE hInstance = GetModuleHandleA(NULL);

	NOTIFYICONDATAA iconData = { 0 };
	iconData.cbSize = sizeof(iconData);
	iconData.hWnd = GetActiveWindow();
	iconData.uID = NOTIFY_ICON_DATA_ID;
	iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	iconData.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_thingy));
	iconData.uCallbackMessage = WM_TRAYICON;
	memcpy(iconData.szTip, WIN_TITLE, sizeof(WIN_TITLE));
	Shell_NotifyIconA(NIM_ADD, &iconData);
}

/*
* Change the system tray icon.
*/
void	change_tray_icon(int IDI_thingy)
{
	HMODULE hInstance = GetModuleHandleA(NULL);

	NOTIFYICONDATAA iconData = { 0 };
	iconData.cbSize = sizeof(iconData);
	iconData.hWnd = GetActiveWindow();
	iconData.uID = NOTIFY_ICON_DATA_ID;
	iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	iconData.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_thingy));
	iconData.uCallbackMessage = WM_TRAYICON;
	memcpy(iconData.szTip, WIN_TITLE, sizeof(WIN_TITLE));
	Shell_NotifyIconA(NIM_MODIFY, &iconData);
}

/*
* Remove the system tray icon.
*/
void	remove_tray_icon()
{
	NOTIFYICONDATAA iconData = { 0 };
	iconData.cbSize = sizeof(iconData);
	iconData.hWnd = GetActiveWindow();
	iconData.uID = NOTIFY_ICON_DATA_ID;
	Shell_NotifyIconA(NIM_DELETE, &iconData);
}

/*
* Set the window's minimum size based on desired client area, NOT full window size!
* Unused because I figured out subclassing, but I'll keep it herebecause it's a useful trick.
*/
static void	set_Win32_min_window_size(HWND hWnd, MINMAXINFO *mmi, int min_width, int min_height)
{
	// Get window styles
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	// Calculate required window size for desired client area
	RECT window_size = { 0, 0, WIN_WIDTH, WIN_HEIGHT };
	AdjustWindowRectEx(&window_size, style, FALSE, exStyle);
	// Set window size to client area + window decorations.
	// GLFW asks for client area, Win32 asks for window area (client + decorations)
	mmi->ptMinTrackSize.x = window_size.right - window_size.left;
	mmi->ptMinTrackSize.y = window_size.bottom - window_size.top;
}

std::wstring RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value)
{
	DWORD dataSize{};
	LONG retCode = ::RegGetValueW(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		nullptr,
		&dataSize
	);
	if (retCode != ERROR_SUCCESS)
		throw std::runtime_error("Cannot read string from registry");
	std::wstring data;
	data.resize(dataSize / sizeof(wchar_t));
	retCode = ::RegGetValueW(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		&data[0],
		&dataSize
	);
	if (retCode != ERROR_SUCCESS)
		throw std::runtime_error("Cannot read string from registry");
	data.resize(dataSize / sizeof(wchar_t) - 1);
	return data;
}

int	RegGetString(HKEY hKey, const std::wstring &subKey, const std::wstring &value,
	std::wstring &outstr)
{
	DWORD dataSize{};
	LONG retCode = ::RegGetValueW(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		nullptr,
		&dataSize
	);
	if (retCode != ERROR_SUCCESS)
		return 1;
	outstr.resize(dataSize / sizeof(wchar_t));
	retCode = ::RegGetValueW(
		hKey,
		subKey.c_str(),
		value.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		&outstr[0],
		&dataSize
	);
	if (retCode != ERROR_SUCCESS)
		return 2;
	outstr.resize(dataSize / sizeof(wchar_t) - 1);
	return 0;
}
