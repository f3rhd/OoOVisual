// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include "../../vendor/ImGui/imgui.h"
#include <algorithm>
#include <cfloat>
#include <Core/DCache/DCache.h>
#include <Core/Execution/ExecutionStage.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/Types/Types.h>
#include <vector>
#include <Visualizer/Camera.h>
#include <Visualizer/DrawElement.h>
#include <Visualizer/DrawElementExecutionUnit.h>
#include <Visualizer/Utils.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Draw_Element_Execution_Unit::show_tooltip() const {
			const Core::Reservation_Station_Entry* issued_entry{};
			switch (_id) {
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::ADDER_SUBTRACTOR_UNIT:
				issued_entry = Core::Execution_Stage::issued()[0];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::BITWISE_UNIT:
				issued_entry = Core::Execution_Stage::issued()[1];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::LESS_THAN_SETTER_UNIT:
				issued_entry = Core::Execution_Stage::issued()[2];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::MULTIPLIER_UNIT:
				issued_entry = Core::Execution_Stage::issued()[3];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::DIVIDER_UNIT:
				issued_entry = Core::Execution_Stage::issued()[4];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::LOAD_STORE_UNIT:
				issued_entry = Core::Execution_Stage::issued()[5];
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::BRANCH_UNIT:
				issued_entry = Core::Execution_Stage::issued()[6];
				break;
			default:
				break;
			}
			ImGui::BeginTooltip();

			bool is_busy{ issued_entry != nullptr };

			ImGui::Text("Execution Unit Status");
			ImGui::Separator();

			if (ImGui::BeginTable("##ExecUnitTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Status:");
				ImGui::TableSetColumnIndex(1);
				if (is_busy) {
					ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "BUSY");
				}
				else {
					ImGui::TextColored(ImVec4{ 0.4f, 1.0f, 0.4f, 1.0f }, "IDLE");
				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Operation:");
				ImGui::TableSetColumnIndex(1);
				if (is_busy) {
					ImGui::Text("%s", Utils::execution_mode_to_str(issued_entry->mode));
				}
				else {
					ImGui::TextDisabled("N/A");
				}

				if (is_busy) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Fetched at:");
					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(ImVec4{ 0.7f, 0.9f, 1.0f, 1.0f }, "%u", issued_entry->timestamp);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("ROB Index:");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("#%lu", issued_entry->reorder_buffer_entry_index);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("PC:");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("0x%08X", issued_entry->instruction_address);
				}

				ImGui::EndTable();
			}

			ImGui::EndTooltip();
		}
		void Draw_Element_Execution_Unit::show_detailed() const {
			const auto& load_buf{ Core::Execution_Unit_Load_Store::load_buffer() };
			const auto& store_buf{ Core::Execution_Unit_Load_Store::store_buffer() };
			const auto& mem_map{ Core::DCache::cache() };

			ImVec2 min_size{ 200, 200.0f };
			ImVec2 max_size{ FLT_MAX, FLT_MAX };
			ImGuiWindowFlags window_flags{ ImGuiWindowFlags_NoSavedSettings };
			ImGui::SetNextWindowSizeConstraints(min_size, max_size);
			ImGui::SetNextWindowSize({ 900, 600 }, ImGuiCond_FirstUseEver);

			if (ImGui::Begin("Load-Store details", nullptr, window_flags)) {

				ImGui::SeparatorText("internal unit buffers");
				if (ImGui::BeginChild("buffer_section", { 0, 250 }, true)) {

					auto render_buf_lambda = [&](const char* label, const auto& buffer) {
						ImGui::Text("%s", label);
						ImGuiTableFlags table_flags{ ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable };

						if (ImGui::BeginTable(label, 7, table_flags, { 0, 100 })) {
							ImGui::TableSetupColumn("mode");
							ImGui::TableSetupColumn("ts");
							ImGui::TableSetupColumn("reg");
							ImGui::TableSetupColumn("rob");
							ImGui::TableSetupColumn("data");
							ImGui::TableSetupColumn("addr");
							ImGui::TableSetupColumn("tag");
							ImGui::TableHeadersRow();

							for (const auto& entry : buffer) {
								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("%s", Utils::execution_mode_to_str(entry.mode));
								ImGui::TableSetColumnIndex(1); ImGui::Text("%u", entry.timestamp);
								ImGui::TableSetColumnIndex(2); ImGui::Text("r%u", (u32)entry.register_id);
								ImGui::TableSetColumnIndex(3); ImGui::Text("%zu", entry.reorder_buffer_entry_index);
								ImGui::TableSetColumnIndex(4); ImGui::Text("0x%X", (u32)entry.register_data.SIGNED);
								ImGui::TableSetColumnIndex(5); ImGui::Text("0x%X", (u32)entry.calculated_address);
								ImGui::TableSetColumnIndex(6); ImGui::Text("#%u", entry.producer_tag);
							}
							ImGui::EndTable();
						}
						};

					render_buf_lambda("load buffer", load_buf);
					ImGui::Spacing();
					render_buf_lambda("store buffer", store_buf);

					ImGui::EndChild();
				}

				ImGui::Spacing();

				ImGui::SeparatorText("instruction cache & memory");
				if (ImGui::BeginChild("memory_section", { 0, 0 }, true)) {

					std::vector<memory_addr_t> sorted_keys{};
					for (const auto& [addr, val] : mem_map) {
						sorted_keys.push_back(addr);
					}
					std::sort(sorted_keys.begin(), sorted_keys.end());

					ImGuiTableFlags mem_table_flags{ ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY };

					if (ImGui::BeginTable("mem_table", 2, mem_table_flags)) {
						ImGui::TableSetupColumn("address", ImGuiTableColumnFlags_WidthFixed, 150.0f);
						ImGui::TableSetupColumn("data (hex / dec)", ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableHeadersRow();

						ImGuiListClipper clipper{};
						clipper.Begin(static_cast<int>(sorted_keys.size()));
						while (clipper.Step()) {
							for (int i{ clipper.DisplayStart }; i < clipper.DisplayEnd; i++) {
								auto addr{ sorted_keys[i] };
								auto val{ mem_map.at(addr) };

								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("0x%08X", addr);
								ImGui::TableSetColumnIndex(1);
								ImGui::Text("0x%02X (%u)", (u32)val, (u32)val);
							}
						}
						ImGui::EndTable();
					}
					ImGui::EndChild();
				}
			}
			ImGui::End();
		}
		void Draw_Element_Execution_Unit::draw(const Camera& cam) {
			if (is_hovered(cam))
				show_tooltip();
			show_architectural(cam);
			if (_id == DRAW_ELEMENT_ID::LOAD_STORE_UNIT) {
				set_detailed(cam);
				if (_detailed)
					show_detailed();
			}
		}
	}
}