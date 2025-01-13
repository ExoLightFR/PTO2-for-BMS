#include "PTO2_for_BMS.hpp"

#include <utility>
#include <string>
#include <array>
#include <algorithm>

#include <Windows.h>

#include "hidapi.h"
#include "imgui.h"
#include "GLFW/glfw3.h"

static std::pair<uint8_t, uint8_t> decompose_release_number(uint16_t release_number)
{
	return {
		release_number >> 8,
		release_number & 0xff
	};
}

template <size_t N>
static std::string get_supported_firmware_str(std::array<uint16_t, N> const &firmwares)
{
	std::string outstr;
	// Each item should be at most 6 chars including coma and space (e.g. "1.05, ") + null-terminator
	outstr.reserve((N * 6) + 1);
	for (uint16_t version : firmwares)
	{
		auto [major, minor] = decompose_release_number(version);
		// Should not dynamically allocate since we reserved earlier
		std::format_to(std::back_inserter(outstr), "{}.{:02}, ", major, minor);
	}
	outstr.erase(outstr.size() - 2);
	return outstr;
}

namespace widgets {

	/*
	* WIP
	*
	* Popup modal to warn user about firmware version mismatch.
	* Since there is no guarantee that a firmware update of the PTO2 doesn't change the protocol
	* for LED control, we have to ensure a whitelist of firmware versions.
	* It's very unlikely that Winwing would change such a thing, but technically not impossible.
	* According to hid-api's readme, an "random" hid_write() could damage a device.
	* Thus, I decide to err on the side of caution, but leave the option to override.
	*/
	void	PTO2_firmware_warning_modal()
	{
		constexpr auto supported_firmware = std::to_array<uint16_t>({ 0x104 });
		static_assert(supported_firmware.size()
			&& std::is_sorted(supported_firmware.begin(), supported_firmware.end()));
		static std::string supported_firmware_str = get_supported_firmware_str(supported_firmware);

		if (!g_context.hid_device)
			return;

		uint16_t release_number = hid_get_device_info(g_context.hid_device)->release_number;
		static bool open_modal = !std::count(supported_firmware.begin(), supported_firmware.end(),
			release_number);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ImGui::GetStyle().FrameRounding);
		if (open_modal)
			ImGui::OpenPopup("FIRMWARE VERSION WARNING");

		float width_avail = ImGui::GetMainViewport()->Size.x;

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowSize({ width_avail * 0.9f, 0 });
		ImGui::SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(0x7f, 0, 0, 0xff));
		if (ImGui::BeginPopupModal("FIRMWARE VERSION WARNING", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			// TODO: change color of "damage" or just go fullcaps lmao
			// https://github.com/ocornut/imgui/issues/902
			// https://github.com/ocornut/imgui/issues/2313
			auto [major, minor] = decompose_release_number(release_number);
			ImGui::TextWrapped("Your Winwing PTO2's firmware version is %u.%02u, which has not been tested"
				" yet. Although unlikely, this app could be incompatible"
				" with your PTO2 or even DAMAGE its firmware.",
				major, minor);

			if (release_number > supported_firmware.back())
			{
				ImGui::TextWrapped("It looks like your PTO2 firmware is too recent for this app. Consider"
					" downloading the latest version of 'PTO2 for BMS' from GitHub and trying again.");
			}
			else
			{
				ImGui::TextWrapped("Please close this app, update your PTO2 firmware through SimAppPro,"
					" and try again.");
			}
			ImGui::Separator();
			ImGui::TextWrapped("Supported firmware versions: %s", supported_firmware_str.c_str());

			// === OVERRIDE CHECKBOX ===
			ImGui::Separator();
			static bool override = false;
			ImGui::Checkbox("I understand the risk, proceed anyway", &override);

			// === MODAL RIGHT-ALIGNED BUTTONS ===
			auto calc_button_size = [](const char *txt) {
				return ImGui::CalcTextSize(txt).x + ImGui::GetStyle().FramePadding.x * 2;
				};

			// Track cursor x position for right-alignment
			float cursor_x = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x;

			// Download latest version button (only when relevant)
			if (release_number > supported_firmware.back())
			{
				cursor_x -= calc_button_size("Download latest app update");
				ImGui::SetCursorPosX(cursor_x);
				if (ImGui::Button("Download latest app update"))
				{
					ShellExecuteA(NULL, "open", "https://github.com/ExoLightFR/PTO2-for-BMS/releases/latest",
						NULL, NULL, SW_SHOWNORMAL);
				}
				ImGui::SetItemTooltip("https://github.com/ExoLightFR/PTO2-for-BMS/releases/latest");
				if (ImGui::IsItemHovered())
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				ImGui::SameLine();
				cursor_x -= ImGui::GetStyle().ItemSpacing.x;
			}

			// Quit app button
			cursor_x -= calc_button_size("Quit app");
			ImGui::SetCursorPosX(cursor_x);
			if (ImGui::Button("Quit app"))
				glfwSetWindowShouldClose(g_context.glfw_window, GLFW_TRUE);

			// === MODAL LEFT-ALIGNED "RUN ANYWAY" BUTTON ===
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);

			if (override && widgets::ColoredButton("Run anyway", { 127, 0, 0 }))
			{
				open_modal = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

}
