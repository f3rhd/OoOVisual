// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementCoreUnit.h>
#include <Visualizer/Constants.h>
#include <Visualizer/Utils.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		bool Draw_Element_Core_Unit::is_hovered(const Camera& cam) const {
			ImVec2 p1{ cam.world_to_screen(_position) };
			ImVec2 p2{ cam.world_to_screen(ImVec2{ _position.x + _dimension.x, _position.y + _dimension.y }) };

			return ImGui::IsMouseHoveringRect(p1, p2);
		}

		void Draw_Element_Core_Unit::set_detailed(const Camera& cam) {

			if (is_hovered(cam) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				_detailed = true;
			}
			if (is_hovered(cam) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right)) {
				_detailed = false;
			}
		}

		static bool is_on_screen(ImVec2 p1, ImVec2 p2) {
			ImVec2 clip_min{ ImGui::GetWindowPos() };
			ImVec2 clip_max{ clip_min.x + ImGui::GetWindowSize().x,
							 clip_min.y + ImGui::GetWindowSize().y };

			if (p2.x < clip_min.x || p1.x > clip_max.x ||
				p2.y < clip_min.y || p1.y > clip_max.y) {
				return false;
			}

			return true;
		}
		void Draw_Element_Core_Unit::show_architectural(const Camera& cam) const {
			ImVec2 p1{ cam.world_to_screen(_position) };
			ImVec2 p2{ cam.world_to_screen(ImVec2{ _position.x + _dimension.x, _position.y + _dimension.y }) };

			if (!is_on_screen(p1, p2))
				return;

			ImDrawList* draw_list{ ImGui::GetWindowDrawList() };

			if (is_hovered(cam)) {
				Utils::draw_glow_rec(p1, ImVec2{ p2.x - p1.x, p2.y - p1.y }, Constants::BLACK);
			}

			draw_list->AddRectFilled(p1, p2, Constants::BLACK, 10.0f);
			draw_list->AddRect(p1, p2, Constants::WHITE, 10.0f);

			const char* text{ "" };
			switch (_id) {
			case DRAW_ELEMENT_ID::FETCH_UNIT:            text = "FETCH"; break;
			case DRAW_ELEMENT_ID::FETCH_GROUP:           text = "FETCH GROUP"; break;
			case DRAW_ELEMENT_ID::DISPATCHER:            text = "DISPATCHER"; break;
			case DRAW_ELEMENT_ID::STATION_0:             text = "STATION 0"; break;
			case DRAW_ELEMENT_ID::STATION_1:             text = "STATION 1"; break;
			case DRAW_ELEMENT_ID::STATION_2:             text = "STATION 2"; break;
			case DRAW_ELEMENT_ID::STATION_3:             text = "STATION 3"; break;
			case DRAW_ELEMENT_ID::STATION_4:             text = "STATION 4"; break;
			case DRAW_ELEMENT_ID::STATION_5:             text = "STATION 5"; break;
			case DRAW_ELEMENT_ID::STATION_6:             text = "STATION 6"; break;
			case DRAW_ELEMENT_ID::ADDER_SUBTRACTOR_UNIT: text = "ADDER\nSUBTRACTOR"; break;
			case DRAW_ELEMENT_ID::BITWISE_UNIT:          text = "BITWISE"; break;
			case DRAW_ELEMENT_ID::LESS_THAN_SETTER_UNIT: text = "SET-LESS"; break;
			case DRAW_ELEMENT_ID::MULTIPLIER_UNIT:       text = "MULTIPLIER"; break;
			case DRAW_ELEMENT_ID::DIVIDER_UNIT:          text = "DIVIDER"; break;
			case DRAW_ELEMENT_ID::LOAD_STORE_UNIT:       text = "LOAD-STORE"; break;
			case DRAW_ELEMENT_ID::BRANCH_UNIT:           text = "BRANCH"; break;
			case DRAW_ELEMENT_ID::REGISTER_MANAGER:      text = "REGISTER MANAGER"; break;
			case DRAW_ELEMENT_ID::REORDER_BUFFER:        text = "REORDER BUFFER"; break;
			default: break;
			}

			float scaledFontSize{ ImGui::GetFontSize() * 2 * cam.zoom };

			ImVec2 textSize{ ImGui::GetFont()->CalcTextSizeA(scaledFontSize, FLT_MAX, 0.0f, text) };

			ImVec2 screenCenter{ (p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f };
			ImVec2 textPos{ screenCenter.x - (textSize.x / 2.0f), screenCenter.y - (textSize.y / 2.0f) };

			draw_list->AddText(ImGui::GetFont(), scaledFontSize, textPos, Constants::WHITE, text);
		}
	}
}