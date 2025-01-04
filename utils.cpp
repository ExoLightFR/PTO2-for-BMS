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