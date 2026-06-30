// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementReorderBuffer.h>
#include <Visualizer/Constants.h>
#include <Visualizer/Utils.h>
#include <Core/Commit/ReorderBuffer.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Draw_Element_Reorder_Buffer::show_tooltip() const {
			ImGui::BeginTooltip();
			ImGui::Text("Head: %ld", Core::Reorder_Buffer::head());
			ImGui::Text("Tail: %ld", Core::Reorder_Buffer::tail());
			if (Core::Reorder_Buffer::full()) {
				ImGui::PushStyleColor(ImGuiCol_Text, Constants::RED);
				ImGui::Text("Reorder buffer is full.");
				ImGui::PopStyleColor();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, Constants::GREEN);
				ImGui::Text("Reorder buffer is not full.");
				ImGui::PopStyleColor();
			}
			ImGui::EndTooltip();
		}

		void Draw_Element_Reorder_Buffer::show_detailed() const {
			static bool follow_head{ true };
			ImGuiIO& io{ ImGui::GetIO() };

			float sidebar_width{ 450.0f };
			ImGui::SetNextWindowSize(ImVec2{ sidebar_width, io.DisplaySize.y * 0.8f }, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2{ io.DisplaySize.x - sidebar_width - 20.0f, 20.0f }, ImGuiCond_FirstUseEver);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.05f, 0.05f, 0.05f, 0.85f });

			ImGuiWindowFlags window_flags{ ImGuiWindowFlags_HorizontalScrollbar };

			if (ImGui::Begin("Reorder Buffer Status", nullptr, window_flags)) {

				ImGui::Text("ROB Internals");
				ImGui::SameLine(ImGui::GetWindowWidth() - 120.0f);
				ImGui::Checkbox("Follow Head", &follow_head);
				ImGui::Separator();

				// Table styling
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
				ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4{ 1.0f, 1.0f, 1.0f, 0.05f });

				ImGuiTableFlags flags{ ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
									   ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX |
									   ImGuiTableFlags_Resizable };

				if (ImGui::BeginTable("ROB_Scroll_Table", 5, flags, ImGui::GetContentRegionAvail())) {
					ImGui::TableSetupScrollFreeze(1, 1);

					ImGui::TableSetupColumn("Idx", ImGuiTableColumnFlags_WidthFixed, 40.0f);
					ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
					ImGui::TableSetupColumn("Ready", ImGuiTableColumnFlags_WidthFixed, 50.0f);
					ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthFixed, 250.0f);
					ImGui::TableSetupColumn("Ptr", ImGuiTableColumnFlags_WidthFixed, 60.0f);
					ImGui::TableHeadersRow();

					for (int i{ 0 }; i < Core::Constants::REORDER_BUFFER_SIZE; ++i) {
						auto* entry{ Core::Reorder_Buffer::buffer()[i].get() };
						ImGui::TableNextRow();

						bool is_head{ i == Core::Reorder_Buffer::head() };
						bool is_tail{ i == Core::Reorder_Buffer::tail() };

						// Visual cues for the circular buffer pointers
						if (is_head) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Constants::REORDER_BUFFER_HEAD_COLOR);
							if (follow_head) ImGui::SetScrollHereY(0.5f);
						}
						else if (is_tail) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, Constants::REORDER_BUFFER_TAIL_COLOR);
						}

						ImGui::TableSetColumnIndex(0); ImGui::Text("%02d", i);

						ImGui::TableSetColumnIndex(1);
						if (entry) {
							ImGui::Text("%s", Utils::flowtype_to_str(entry->flow_type));
						}
						else {
							ImGui::TextDisabled("-");
						}

						ImGui::TableSetColumnIndex(2);
						if (entry) {
							if (entry->ready) ImGui::TextColored(ImVec4{ 0.2f, 1.0f, 0.2f, 1.0f }, "YES");
							else ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "NO");
						}

						ImGui::TableSetColumnIndex(3);
						if (entry) {
							render_entry_details(entry);
						}

						ImGui::TableSetColumnIndex(4);
						if (is_head && is_tail) ImGui::Text("H/T");
						else if (is_head) ImGui::Text("HEAD");
						else if (is_tail) ImGui::Text("TAIL");
					}
					ImGui::EndTable();
				}
				ImGui::PopStyleColor(2);
			}
			ImGui::End();
			ImGui::PopStyleColor();
		}
		void Draw_Element_Reorder_Buffer::render_entry_details(Core::Reorder_Buffer_Entry* entry) const {
			if (!entry) {
				ImGui::TextDisabled("EMPTY");
				return;
			}

			ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "[TS: %u]", entry->self_timestamp);
			ImGui::SameLine();

			if (entry->ready) {
				ImGui::TextColored(ImVec4{ 0.2f, 1.0f, 0.2f, 1.0f }, "[READY]");
			}
			else {
				ImGui::TextColored(ImVec4{ 1.0f, 0.8f, 0.2f, 1.0f }, "[BUSY]");
			}
			ImGui::SameLine();
			ImGui::Text("|");
			ImGui::SameLine();

			if (const auto* reg{ dynamic_cast<Core::Register_Reorder_Buffer_Entry*>(entry) }) {
				ImGui::Text("Arch Reg: %d | Alias: %d -> %d",
							reg->architectural_register_id, reg->old_alias, reg->new_alias);
			}
			else if (const auto* load{ dynamic_cast<Core::Load_Reorder_Buffer_Entry*>(entry) }) {
				ImGui::Text("Arch Reg: %d | Alias: %d -> %d",
							load->architectural_register_id, load->old_alias, load->new_alias);
				if (load->misspeculated) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "MISPREDICTED!");
				}
			}
			else if (const auto* store{ dynamic_cast<Core::Store_Reorder_Buffer_Entry*>(entry) }) {
				ImGui::Text("Store ID: %u", store->store_id);
			}
			else if (const auto* cond_br{ dynamic_cast<Core::Branch_Conditional_Reorder_Buffer_Entry*>(entry) }) {
				if (cond_br->mispredicted) {
					ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "MISPREDICTED!");
				}
				else {
					ImGui::TextDisabled("Predict OK");
				}
			}
			else if (const auto* uncond_br{ dynamic_cast<Core::Branch_Unconditional_Reorder_Buffer_Entry*>(entry) }) {
				ImGui::Text("Arch: %d | Alias: %d",
							uncond_br->architectural_register_id, uncond_br->new_alias);
				if (uncond_br->mispredicted) {
					ImGui::SameLine();
					ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "MISPREDICTED!");
				}
			}
			else {
				ImGui::Text("Base/Unknown Entry");
			}
		}
		void Draw_Element_Reorder_Buffer::draw(const Camera& cam) {

			if (is_hovered(cam))
				show_tooltip();
			show_architectural(cam);
			set_detailed(cam);
			if (_detailed)
				show_detailed();
		}

	} // namespace Visualizer

} // namespace OoOVisual
