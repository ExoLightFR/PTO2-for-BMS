// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <windows.h>
#include <WinUser.h>
#include <shellapi.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // Will drag system OpenGL headers

#include <thread>

#include "resource.h"
#include "PTO2_for_BMS.hpp"

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

/*
* Static variables to access GLFW window and WndProc handles inside our own WndProc function.
* This allows subclassing GLFW's WndProc function, so we can handle messages ourselves for
* the system tray icon. See below.
*/
static WNDPROC		s_glfw_wndproc = nullptr;

/*
* Write our own WndProc function that subclasses GLFW's. With this, we can handle our own custom events,
* like the system tray icon messages, and let GLFW do everything else.
*/
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED)
		{
			// Hide the window with an animation (so user doesn't think it crashed or quit)
			AnimateWindow(hWnd, 100, AW_HIDE | AW_SLIDE | AW_VER_NEGATIVE);
			return FALSE;
		}
		else
			return CallWindowProc(s_glfw_wndproc, hWnd, uMsg, wParam, lParam);
	}
	case WM_DPICHANGED:
	{
		// Set ImGui app scaling (style & font)
		UINT new_dpi = HIWORD(wParam);
		set_ImGui_scaling_from_DPI(new_dpi);
		// Disable window size limits to allow downscaling
		glfwSetWindowSizeLimits(g_context.glfw_window, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE);
		// Subclass GLFW's handling of WM_DPICHANGED
		LRESULT retval = CallWindowProc(s_glfw_wndproc, hWnd, uMsg, wParam, lParam);
		// Fetch the newly set window size and set it as the new minimum
		int win_width, win_height;
		glfwGetWindowSize(g_context.glfw_window, &win_width, &win_height);
		glfwSetWindowSizeLimits(g_context.glfw_window, win_width, win_height, GLFW_DONT_CARE, GLFW_DONT_CARE);
		// Return result of GLFW's WndProc call
		return retval;
	}
	/*
	* This was a bit of a pain to research. Here are some resources I've used:
	* https://www.codeproject.com/Articles/18783/Example-of-a-SysTray-App-in-Win32
	* https://www.lotushints.com/2013/03/win32-hide-to-system-tray-part-1/ (also check out parts 2 & 3)
	* https://www.oocities.org/technofundo/tech/win/SNIcon.html
	* https://codingmisadventures.wordpress.com/2009/02/20/creating-a-system-tray-icon-using-visual-c-and-win32/
	*/
	case WM_TRAYICON: // Custom message for our tray icon
	{
		if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			ShowWindow(hWnd, SW_RESTORE);
			SetForegroundWindow(hWnd);
			SetFocus(hWnd);
			return TRUE;
		}
		else if (LOWORD(lParam) == WM_RBUTTONUP)
		{
			POINT clickPoint = {};
			UINT_PTR id = 123;

			UINT flags = MF_BYPOSITION | MF_STRING;
			GetCursorPos(&clickPoint);
			HMENU menu = CreatePopupMenu();

			UINT connected_flag = (g_context.thread_running ? MF_CHECKED : MF_UNCHECKED);
			InsertMenuA(menu, 0xFFFFFFFF, flags | connected_flag, ID_TRAY_MENU_CONNECT, "Connect to BMS");
			InsertMenuA(menu, 0xFFFFFFFF, flags, ID_TRAY_MENU_MINIMIZE, "Minimize to tray");
			InsertMenuA(menu, 0xFFFFFFFF, flags, ID_TRAY_MENU_SEPARATOR, NULL);
			InsertMenuA(menu, 0xFFFFFFFF, flags, ID_TRAY_MENU_QUIT, "Quit");

			// Makes the connect option bold
			SetMenuDefaultItem(menu, ID_TRAY_MENU_CONNECT, FALSE);
			
			// This is annoying, but without this the menu doesn't disappear when clicking outside it.
			// Just a Win32 limitation, I guess? The TrackIR software has the same problem.
			SetForegroundWindow(hWnd);
			TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
				clickPoint.x, clickPoint.y, 0, hWnd, NULL);
			return TRUE;
		}
		else
			return CallWindowProc(s_glfw_wndproc, hWnd, uMsg, wParam, lParam);
	}
	case WM_COMMAND:
	{
		WORD ID = LOWORD(wParam);
		WORD event = HIWORD(wParam);
		switch (ID)
		{
		case ID_TRAY_MENU_CONNECT:
		{
			if (g_context.thread_running) {
				g_context.thread_running = false;
				g_context.thread.join();
			}
			else
				g_context.thread = std::jthread(&thread_routine);
			break;
		}
		case ID_TRAY_MENU_MINIMIZE:
			ShowWindow(hWnd, SW_HIDE);
			break;
		case ID_TRAY_MENU_QUIT:
			glfwSetWindowShouldClose(g_context.glfw_window, GLFW_TRUE);
			break;
		default:
			return CallWindowProc(s_glfw_wndproc, hWnd, uMsg, wParam, lParam);
		}
		return TRUE;
	}
	default:
		// Subclass GLFW's window processing handler
		return CallWindowProc(s_glfw_wndproc, hWnd, uMsg, wParam, lParam);
	}
}

// Main code
int main(void)
{
	// Don't allow multiple instances, out of caution
	if (FindWindowA(NULL, WIN_TITLE) != nullptr)
	{
		MessageBoxA(nullptr, "Another instance of this application is already running! "
			"It might be minimized in your taskbar.",
			"Whoops!", MB_OK | MB_ICONHAND | MB_SETFOREGROUND);
		return 1;
	}

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, nullptr, nullptr);
	if (window == nullptr)
		return 1;
	g_context.glfw_window = window;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	/*
	* Allow window resizing but set minimum size to never crop content.
	* Get the window size first then set it as a limit: this takes into account GLFW's
	* rescaling of the window from the screen's DPIs (GLFW_SCALE_TO_MONITOR set to true)
	*/
	int win_width, win_height;
	glfwGetWindowSize(window, &win_width, &win_height);
	glfwSetWindowSizeLimits(window, win_width, win_height, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
	io.IniFilename = nullptr;								// Disable .ini ImGui file
	
	// Set custom WndProc handler so we can subclass GLFW's
	HWND hWnd = glfwGetWin32Window(window);
	s_glfw_wndproc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
	
	// Set ImGui scale based on the screen's scaling factor (DPIs)
	UINT win_dpi = GetDpiForWindow(hWnd);
	float scale_factor = (float)win_dpi / USER_DEFAULT_SCREEN_DPI;
	ImGui::GetStyle() = get_custom_imgui_style(scale_factor);
	// Load our custom font from memory, taking into account our scaling factor
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(Roboto_Medium_compressed_data_base85,
		std::trunc(17 * scale_factor));

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Init icons to red because app is not connected on startup
	set_window_icon(WINDOW_ICON_ID_RED);
	add_tray_icon(WINDOW_ICON_ID_RED);
	hid_init();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Update the system tray icon based on connection status
		int icon_id = (g_context.thread_running ? WINDOW_ICON_ID_GREEN : WINDOW_ICON_ID_RED);
		change_tray_icon(icon_id);

		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Give CPU cycles back when window is minimized or hidden (to tray, other desktop...)
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0 || !(GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		render_main_window(io);

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Dirty (good?) trick to give CPU cycles back when window is not focused
		if (!glfwGetWindowAttrib(window, GLFW_FOCUSED))
			ImGui_ImplGlfw_Sleep(10);

		glfwSwapBuffers(window);
	}

	if (g_context.thread_running) // Clean thread exit
	{
		g_context.thread_running = false;
		g_context.thread.join();
	}

	hid_close(g_context.hid_device);
	hid_exit();

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	remove_tray_icon();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

// You can also do a trick by setting /ENTRY to "mainCRTStartup", which makes the Windows application
// eventually call main() on entry. Skips the need for this WinMain thingy.
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	return main();
}
