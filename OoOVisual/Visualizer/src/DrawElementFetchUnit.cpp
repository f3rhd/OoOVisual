// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementFetchUnit.h>
#include <Visualizer/Constants.h>
#include <Core/Fetch/Fetch.h>
#include <Core/Types/Types.h>
#include <iostream>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Draw_Element_Fetch_Unit::show_detailed() const {
			ImGuiViewport* viewport{ ImGui::GetMainViewport() };
			float padding{ 20.0f };

			ImVec2 initial_pos{ viewport->WorkPos.x + viewport->WorkSize.x - padding,
								viewport->WorkPos.y + padding };
			ImVec2 window_pivot{ 1.0f, 0.0f };

			ImGui::SetNextWindowPos(initial_pos, ImGuiCond_FirstUseEver, window_pivot);
			ImGui::SetNextWindowBgAlpha(0.9f);

			ImGuiWindowFlags window_flags{ ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings };

			static bool follow_pc{ true };
			if (ImGui::Begin("Fetch Unit Details", nullptr, window_flags)) {

				ImGui::TextColored(ImVec4{ 0.4f, 0.7f, 1.0f, 1.0f }, "FETCH UNIT INTERNALS");
				ImGui::Separator();
				ImGui::Text("Pc: %u", Core::Fetch_Unit::program_counter());
				ImGui::Separator();
				ImGui::Checkbox("Follow Pc", &follow_pc);

				if (ImGui::CollapsingHeader("Instruction Stream", ImGuiTreeNodeFlags_DefaultOpen)) {

					ImGuiWindowFlags child_flags = ImGuiWindowFlags_HorizontalScrollbar;
					if (ImGui::BeginChild("InstructionScroll", ImVec2{ 350.0f, 250.0f }, true, child_flags)) {
						const auto& stream{ Core::Fetch_Unit::instruction_stream() };
						const memory_addr_t current_pc{ Core::Fetch_Unit::program_counter() };

						if (follow_pc) {

							int pc_row_index{ -1 };
							for (size_t i{ 0 }; i < stream.size(); ++i) {
								if (static_cast<memory_addr_t>(stream[i].second) == current_pc) {
									pc_row_index = static_cast<int>(i);
									break;
								}
							}

							if (follow_pc && pc_row_index != -1) {
								float item_height{ ImGui::GetTextLineHeightWithSpacing() };

								float target_scroll_y{ (pc_row_index * item_height) - (ImGui::GetWindowHeight() * 0.5f) + (item_height * 0.5f) };

								ImGui::SetScrollY(target_scroll_y);
							}
						}

						if (ImGui::BeginTable("InstructionTable", 2, ImGuiTableFlags_SizingFixedFit)) {
							ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 50.0f);
							ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthStretch);

							ImGuiListClipper clipper;
							clipper.Begin(static_cast<int>(stream.size()));

							while (clipper.Step()) {
								for (int i{ clipper.DisplayStart }; i < clipper.DisplayEnd; ++i) {
									ImGui::TableNextRow(); // Start a new row for the table

									bool is_current_line{ static_cast<memory_addr_t>(stream[i].second) == current_pc };

									if (is_current_line) {
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 0.8f, 0.0f, 1.0f });
									}

									if (stream[i].first.back() == ':') {
										ImGui::TableSetColumnIndex(1);
										ImGui::Text("%s", stream[i].first.c_str());
									}
									else {
										ImGui::TableSetColumnIndex(0);
										ImGui::TextDisabled("[%04ld]", stream[i].second);

										ImGui::TableSetColumnIndex(1);
										ImGui::Text("%s", stream[i].first.c_str());
									}

									if (is_current_line) {
										ImGui::PopStyleColor();
									}
								}
							}
							ImGui::EndTable();
						}
					}
					ImGui::EndChild();
				}
			}
			ImGui::End();
		}
		void Draw_Element_Fetch_Unit::show_tooltip() const {
			auto current_pc{ Core::Fetch_Unit::program_counter() };
			ImGui::SetTooltip(
				"Fetch Unit\nPc: %u"
				, current_pc
			);
		}
		void Draw_Element_Fetch_Unit::draw(const Camera& cam) {
			if (is_hovered(cam))
				show_tooltip();
			show_architectural(cam);
			set_detailed(cam);
			if (_detailed)
				show_detailed();
		}
	}
}

