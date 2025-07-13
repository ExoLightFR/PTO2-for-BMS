#include "PTO2_for_BMS.hpp"

namespace widgets {

void	connect_button()
{
	float height = ImGui::GetFrameHeight() * 2.0f;

	if (!g_context.hid_device)
	{
		// No PTO2 device found, don't allow connection to BMS
		ImGui::BeginDisabled();
		ColoredButton("No Winwing PTO2 found", { 127, 127, 127 }, { -1, height });
		ImGui::EndDisabled();
	}
	else
	{
		// PTO2 has been found
		if (g_context.thread_running)
		{
			ImColor color = (g_context.retro_mode ? ImColor{ 127, 127, 127 } : ImColor{ 172, 0, 0 });
			if (ColoredButton("Disconnect from BMS", color, { -1, height }))
			{
				g_context.thread_running = false;
				g_context.thread.join();
			}
			set_window_icon(WINDOW_ICON_ID_GREEN);
		}
		else
		{
			ImColor color = (g_context.retro_mode ? ImColor{ 127, 127, 127 } : ImColor{ 0, 172, 0 });
			if (ColoredButton("Connect to BMS", color, { -1, height }))
			{
				g_context.thread = std::jthread(&thread_routine);
			}
			set_window_icon(WINDOW_ICON_ID_RED);
		}
	}
}

/*
* Row of several horizontal buttons to select which config file to use. One button for
* each detected BMS install. Returns whether or not a button was clicked.
*/
bool	BMS_version_select()
{
	bool clicked = false;

	auto button = [](const char *label, bool selected, float width)
		{
			ImGuiCol color = ImGui::GetColorU32(ImGuiCol_ButtonActive);
			if (selected)
				return widgets::ColoredButton(label, color, { width, 0 });
			else
				return ImGui::Button(label, { width, 0 });
		};

	ImGui::BeginDisabled(g_context.thread_running || g_context.conf_file_paths.size() == 1);
	if (g_context.conf_file_paths.size() == 1)
	{
		ImGui::Button("No BMS installation found, configuration file is in current folder", { -1, 0 });
	}
	else
	{
		// Set x-spacing to y-spacing to be consistent
		ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, ImGui::GetStyle().ItemSpacing.y);
		size_t num_of_installs = g_context.conf_file_paths.size() - 1;
		// Calculate individual button width from content region & x-spacing of buttons
		float width_avail = ImGui::GetContentRegionAvail().x - (ImGui::GetStyle().ItemSpacing.x * (num_of_installs - 1));
		float button_width = width_avail / num_of_installs;
		for (size_t i = 0; i < num_of_installs; ++i)
		{
			std::string const &install_name = g_context.conf_file_paths[i].install_name;
			// Because of sub-pixel width shenanigans, just let ImGui handle the width of the last button
			if (i == num_of_installs - 1)
				button_width = -1;
			if (button(install_name.c_str(), g_context.selected_conf_file_idx == i, button_width))
			{
				g_context.selected_conf_file_idx = i;
				clicked = true;
			}
			// Same line except the last button
			if (i != num_of_installs - 1)
				ImGui::SameLine();
		}
		ImGui::PopStyleVar();

	}
	ImGui::EndDisabled();

	return clicked;
}

} // namespace widgets