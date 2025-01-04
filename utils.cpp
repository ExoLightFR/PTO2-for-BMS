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
	style.PopupRounding = 3;

	style.ScaleAllSizes(1.5f);
	ImGui::StyleColorsDark(&style);

	return style;
}

/*
* ImGui button with custom color. Calculates appropriate Hovered and Active colors.
*/
bool	ColoredButton(const char *label, ImColor color, const ImVec2 &size = ImVec2(0, 0))
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