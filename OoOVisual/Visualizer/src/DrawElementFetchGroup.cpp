// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementFetchGroup.h>
#include <Core/Fetch/FetchElements.h>
#include <Visualizer/Constants.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Draw_Element_Fetch_Group::show_tooltip() const {
			if (Core::Fetch_Group::group.empty()) {
				ImGui::SetTooltip("Fetch queue is empty.");
				return;
			}
			ImGui::BeginTooltip();
			ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "FETCH QUEUE CONTENTS");
			ImGui::Separator();

			if (ImGui::BeginTable("FetchQueueTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

				ImGui::TableSetupColumn("Slot");
				ImGui::TableSetupColumn("Address");
				ImGui::TableSetupColumn("TS");
				ImGui::TableSetupColumn("Branch");
				ImGui::TableHeadersRow();

				for (u32 i{}; i < Core::Constants::FETCH_WIDTH; i++) {
					const auto& element = Core::Fetch_Group::group[i];

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%d", i);

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("0x%08X", element.address);

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("%u", element.timestamp);

					ImGui::TableSetColumnIndex(3);
					if (element.branch_prediction == Core::Constants::NOT_BRANCH_INSTRUCTION) {
						ImGui::TextDisabled("-");
					}
					else {
						ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "YES");
					}
				}
			}
			ImGui::EndTable();
			ImGui::EndTooltip();
		}
		void Draw_Element_Fetch_Group::draw(const Camera& cam) {
			if (is_hovered(cam))
				show_tooltip();
			show_architectural(cam);
		}
	}
}