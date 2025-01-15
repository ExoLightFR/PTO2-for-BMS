// A very cool implementation which I yoinked from GitHub (CC0/public domain).
// Credit to @idbrii for this awesome widget! I slightly edited it to fit my needs.
// CREDIT: https://github.com/ocornut/imgui/issues/1658#issuecomment-1086193100

#include <vector>
#include <string>
#include <algorithm>

// Built using imgui v1.78 WIP
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

// https://github.com/forrestthewoods/lib_fts/blob/632ca1ea82bdf65688241bb8788c77cb242fba4f/code/fts_fuzzy_match.h
#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"

#include "PTO2_for_BMS.hpp"

namespace widgets {

	static bool sortbysec_desc(const std::pair<int, int> &a, const std::pair<int, int> &b)
	{
		return (b.second < a.second);
	}

	static int index_of_key(
		std::vector<std::pair<int, int> > pair_list,
		int key)
	{
		for (int i = 0; i < pair_list.size(); ++i)
		{
			auto &p = pair_list[i];
			if (p.first == key)
			{
				return i;
			}
		}
		return -1;
	}

	// Copied from imgui_widgets.cpp
	static float CalcMaxPopupHeightFromItemCount(int items_count)
	{
		ImGuiContext &g = *GImGui;
		if (items_count <= 0)
			return FLT_MAX;
		return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
	}

	static int	get_index_of_selected_item(std::optional<FalconLightData> const &PTO2_light_bind,
		std::vector<FalconLightData> const &items)
	{
		if (PTO2_light_bind.has_value())
		{
			for (int i = 0; i < items.size(); ++i)
			{
				if (PTO2_light_bind->ID == items[i].ID)
					return i;
			}
		}
		return -1;
	}

	bool	PTO2_light_assign(const char *label, PTO2LightID PTO_light_ID, int popup_max_height_in_items /*= -1 */)
	{
		using namespace fts;

		ImGuiContext &g = *GImGui;

		ImGuiWindow *window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		const ImGuiStyle &style = g.Style;

		std::vector<FalconLightData> const &items = g_context.falcon_lights;
		auto &PTO2_light_bind = g_context.PTO2_light_assignment_map[PTO_light_ID];

		int items_count = static_cast<int>(items.size());

		static int focus_idx = get_index_of_selected_item(PTO2_light_bind, items);
		static char pattern_buffer[256] = { 0 };

		bool value_changed = false;

		// === ERASE BUTTON ===
		ImColor button_color = (g_context.retro_mode ? ImColor{127, 127, 127} : ImColor{172, 0, 0});
		ImGui::BeginDisabled(!PTO2_light_bind.has_value());
		ImGui::PushID(PTO_light_ID);
		if (ColoredButton("Erase", button_color))
		{
			PTO2_light_bind.reset();
			value_changed = true;
		}
		ImGui::PopID();
		ImGui::EndDisabled();

		ImGui::SameLine();

		// === COMBO ===
		const ImGuiID id = window->GetID(label);
		const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id); // copied from BeginCombo
		const bool is_already_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
		const bool is_filtering = is_already_open && pattern_buffer[0] != '\0';

		int show_count = items_count;

		std::vector<std::pair<int, int> > itemScoreVector;
		if (is_filtering)
		{
			// Filter before opening to ensure we show the correct size window.
			// We won't get in here unless the popup is open.
			for (int i = 0; i < items_count; i++)
			{
				int score = 0;
				bool matched = fuzzy_match(pattern_buffer, items[i].display_name.c_str(), score);
				if (matched)
					itemScoreVector.push_back(std::make_pair(i, score));
			}
			std::sort(itemScoreVector.begin(), itemScoreVector.end(), sortbysec_desc);
			int current_score_idx = index_of_key(itemScoreVector, focus_idx);
			if (current_score_idx < 0 && !itemScoreVector.empty())
			{
				focus_idx = itemScoreVector[0].first;
			}
			show_count = static_cast<int>(itemScoreVector.size());
		}

		// Define the height to ensure our size calculation is valid.
		if (popup_max_height_in_items == -1) {
			popup_max_height_in_items = 8;
		}
		popup_max_height_in_items = ImMin(popup_max_height_in_items, show_count);


		if (!(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
		{
			int items = popup_max_height_in_items + 2; // extra for search bar
			ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(items)));
		}

		const char *preview = PTO2_light_bind ? PTO2_light_bind->display_name.c_str() : "";
		if (!ImGui::BeginCombo(label, preview, ImGuiComboFlags_None))
			return value_changed || false;


		if (!is_already_open)
		{
			focus_idx = get_index_of_selected_item(PTO2_light_bind, items);
			memset(pattern_buffer, 0, IM_ARRAYSIZE(pattern_buffer));
		}

		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(240, 240, 240, 255));
		ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255));
		ImGui::PushItemWidth(-FLT_MIN);
		// Filter input
		if (!is_already_open)
			ImGui::SetKeyboardFocusHere();
		ImGui::InputText("##ComboWithFilter_inputText", pattern_buffer, 256, ImGuiInputTextFlags_AutoSelectAll);

		ImGui::PopStyleColor(2);

		int move_delta = 0;
		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		{
			--move_delta;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		{
			++move_delta;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_PageUp))
		{
			move_delta -= popup_max_height_in_items;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_PageDown))
		{
			move_delta += popup_max_height_in_items;
		}

		if (move_delta != 0)
		{
			if (is_filtering)
			{
				int current_score_idx = index_of_key(itemScoreVector, focus_idx);
				if (current_score_idx >= 0)
				{
					const int count = static_cast<int>(itemScoreVector.size());
					current_score_idx = ImClamp(current_score_idx + move_delta, 0, count - 1);
					focus_idx = itemScoreVector[current_score_idx].first;
				}
			}
			else
			{
				focus_idx = ImClamp(focus_idx + move_delta, 0, items_count - 1);
			}
		}

		// Copied from ListBoxHeader
		// If popup_max_height_in_items == -1, default height is maximum 7.
		float height_in_items_f = (popup_max_height_in_items < 0 ? ImMin(items_count, 7) : popup_max_height_in_items) + 0.0f;
		ImVec2 size;
		size.x = 0.0f;
		size.y = ImGui::GetTextLineHeightWithSpacing() * height_in_items_f + g.Style.FramePadding.y * 2.0f;

		if (ImGui::BeginListBox("##ComboWithFilter_itemList", size))
		{
			for (int i = 0; i < show_count; i++)
			{
				int idx = is_filtering ? itemScoreVector[i].first : i;
				ImGui::PushID((void *)(intptr_t)idx);
				const bool item_selected = (idx == focus_idx);
				const char *item_text = items[idx].display_name.c_str();
				if (ImGui::Selectable(item_text, item_selected))
				{
					value_changed = true;
					PTO2_light_bind = items[idx];
					ImGui::CloseCurrentPopup();
				}

				if (item_selected)
				{
					ImGui::SetItemDefaultFocus();
					// SetItemDefaultFocus doesn't work so also check IsWindowAppearing.
					if (move_delta != 0 || ImGui::IsWindowAppearing())
					{
						ImGui::SetScrollHereY();
					}
				}
				ImGui::PopID();
			}
			ImGui::EndListBox();

			if (ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				value_changed = true;
				PTO2_light_bind = items[focus_idx];
				ImGui::CloseCurrentPopup();
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				value_changed = value_changed || false;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::PopItemWidth();
		ImGui::EndCombo();


		if (value_changed)
			ImGui::MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

} // namespace widgets
