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
