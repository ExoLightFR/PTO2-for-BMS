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
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // Will drag system OpenGL headers

#include <thread>

#include "resource.h"
#include "PTO2_for_BMS.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

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
static GLFWwindow	*s_glfw_window = nullptr;

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
			glfwSetWindowShouldClose(s_glfw_window, GLFW_TRUE);
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
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100 (WebGL 1.0)
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
	// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
	const char* glsl_version = "#version 300 es";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, nullptr, nullptr);
	if (window == nullptr)
		return 1;
	s_glfw_window = window;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	// Allow window resizing but set minimum to never crop content
	glfwSetWindowSizeLimits(window, WIN_WIDTH, WIN_HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = NULL;

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::GetStyle() = get_custom_imgui_style();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(Roboto_Medium_compressed_data_base85, 17);

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
	ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Init icons to red because app is not connected on startup
	set_window_icon(WINDOW_ICON_ID_RED);
	add_tray_icon(WINDOW_ICON_ID_RED);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Set custom WndProc handler so we can subclass GLFW's
	HWND hWnd = glfwGetWin32Window(window);
	s_glfw_wndproc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	// Main loop
#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!glfwWindowShouldClose(window))
#endif
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
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif

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
