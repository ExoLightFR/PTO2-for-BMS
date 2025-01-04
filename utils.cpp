#include <string>
#include <stdexcept>
#include <Windows.h>
#include "imgui.h"

ImGuiStyle	get_custom_imgui_style()
{
	ImGuiStyle	style = ImGui::GetStyle();

	style.FrameRounding = 3;
	// style.ItemSpacing.y = 6;
	style.GrabRounding = 4;

	style.ScaleAllSizes(1.5f);
	ImGui::StyleColorsDark(&style);

	return style;
}

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