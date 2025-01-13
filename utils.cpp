#include <string>
#include <stdexcept>
#include <Windows.h>
#include <shellapi.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "PTO2_for_BMS.hpp"

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

/*
* Change ImGui's style and our custom font's size to match given DPI.
* Standard DPI (100% scale) is considered to be 96.
*/
void	set_ImGui_scaling_from_DPI(UINT new_dpi)
{
	// Set ImGui style scale based on the screen's scaling factor (DPIs)
	float scale_factor = (float)new_dpi / USER_DEFAULT_SCREEN_DPI;
	ImGui::GetStyle() = get_custom_imgui_style(scale_factor);

	// Reload our custom font, taking into account the new scale.
	// https://github.com/ocornut/imgui/issues/6547
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(Roboto_Medium_compressed_data_base85,
		std::trunc(17 * scale_factor));
	io.Fonts->Build();
	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_CreateDeviceObjects();
}

namespace widgets {

	/*
	* ImGui button with custom color. Calculates appropriate Hovered and Active colors.
	*/
	bool	ColoredButton(const char* label, ImColor color, const ImVec2& size)
	{
		float h, s, v;
		ImGui::ColorConvertRGBtoHSV(color.Value.x, color.Value.y, color.Value.z, h, s, v);

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(h, s, v));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(h, s + 0.1f, v + 0.1f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(h, s + 0.2f, v + 0.2f));
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
