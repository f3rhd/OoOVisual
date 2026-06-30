// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include "../../vendor/ImGui/imgui.h"
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/Types/Types.h>
#include <Frontend/Parser/Lookup.h>
#include <Visualizer/Camera.h>
#include <Visualizer/DrawElementRegisterManager.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Draw_Element_Register_Manager::show_tooltip() const {
			ImGui::BeginTooltip();
			ImGui::TextColored(ImVec4{ 0.4f, 0.7f, 1.0f, 1.0f }, "Physical Register File Status");
			ImGui::Separator();

			const auto& prf{ Core::Register_Manager::phyical_register_file() };
			size_t busy_count{ 0 };

			for (const auto& [id, entry] : prf) {
				if (entry.allocated) busy_count++;
			}

			ImGui::Text("Total Physical Registers: %zu", prf.size());
			ImGui::Text("Registers in Use: %zu", busy_count);
			ImGui::ProgressBar(static_cast<float>(busy_count) / prf.size(), ImVec2{ 200.0f, 0.0f });

			ImGui::EndTooltip();
		}


		void Draw_Element_Register_Manager::show_detailed() const {
			ImGuiViewport* viewport{ ImGui::GetMainViewport() };
			float padding{ 30.0f };
			ImVec2 spawn_pos{ padding, viewport->WorkSize.y - 550.0f - padding };

			ImGui::SetNextWindowPos(spawn_pos, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2{ 450.0f, 550.0f }, ImGuiCond_FirstUseEver);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.07f, 0.07f, 0.07f, 0.92f });

			if (ImGui::Begin("Register Management Unit", nullptr)) {
				if (ImGui::BeginTabBar("RegTabs")) {

					if (ImGui::BeginTabItem("Physical Registers (PRF)")) {
						ImGuiTableFlags prf_flags{ ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY };

						if (ImGui::BeginTable("PRF_Table", 3, prf_flags, ImVec2{ 0.0f, -1.0f })) {
							ImGui::TableSetupScrollFreeze(0, 1);
							ImGui::TableSetupColumn("P-Reg", ImGuiTableColumnFlags_WidthFixed, 60.0f);
							ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
							ImGui::TableSetupColumn("Data Value (Hex)");
							ImGui::TableHeadersRow();

							const auto& prf{ Core::Register_Manager::phyical_register_file() };

							for (auto it{ prf.rbegin() }; it != prf.rend(); ++it) {
								const auto& pair{ *it };
								const auto& entry{ pair.second };

								ImGui::TableNextRow();
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("p%u", pair.first);

								ImGui::TableSetColumnIndex(1);
								if (entry.allocated) {
									ImGui::TextColored(ImVec4{ 1.0f, 0.3f, 0.3f, 1.0f }, "BUSY");
								}
								else {
									ImGui::TextColored(ImVec4{ 0.3f, 1.0f, 0.3f, 1.0f }, "READY");
								}

								ImGui::TableSetColumnIndex(2);
								ImGui::Text("0x%016X", entry.data.SIGNED);
							}
							ImGui::EndTable();
						}
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Alias Tables (RAT/RRAT)")) {
						ImGuiTableFlags rat_flags{ ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY };

						// Arch Idx, ABI Name, RAT, RRAT
						if (ImGui::BeginTable("RAT_Compare", 4, rat_flags, ImVec2{ 0.0f, -1.0f })) {
							ImGui::TableSetupScrollFreeze(0, 1);
							ImGui::TableSetupColumn("Arch", ImGuiTableColumnFlags_WidthFixed, 40.0f);
							ImGui::TableSetupColumn("ABI", ImGuiTableColumnFlags_WidthFixed, 50.0f);
							ImGui::TableSetupColumn("Frontend (RAT)");
							ImGui::TableSetupColumn("Retirement (RRAT)");
							ImGui::TableHeadersRow();

							const auto& rat{ Core::Register_Manager::frontend_alias_table() };
							const auto& rrat{ Core::Register_Manager::retirement_alias_table() };

							for (int i{ 31 }; i >= 0; --i) {
								u32 arch_idx{ static_cast<u32>(i) };
								auto abi_name_{ FrontEnd::Lookup::reg_name(arch_idx) };
								auto abi_name{ abi_name_.c_str() };
								ImGui::TableNextRow();

								ImGui::TableSetColumnIndex(0);
								ImGui::Text("x%u", arch_idx);

								ImGui::TableSetColumnIndex(1);
								ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, "%s", abi_name);

								ImGui::TableSetColumnIndex(2);
								if (rat.count(arch_idx)) {
									ImGui::Text("p%u", rat.at(arch_idx));
								}
								else {
									ImGui::TextDisabled("-");
								}

								ImGui::TableSetColumnIndex(3);
								if (rrat.count(arch_idx)) {
									u32 p_spec{ rat.count(arch_idx) ? rat.at(arch_idx) : 999u };
									u32 p_commit{ rrat.at(arch_idx) };

									if (p_spec != p_commit) {
										ImGui::TextColored(ImVec4{ 1.0f, 0.8f, 0.2f, 1.0f }, "p%u", p_commit);
									}
									else {
										ImGui::Text("p%u", p_commit);
									}
								}
							}
							ImGui::EndTable();
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
			}
			ImGui::End();
			ImGui::PopStyleColor();
		}
		void Draw_Element_Register_Manager::draw(const Camera& cam) {

			if (is_hovered(cam))
				show_tooltip();
			show_architectural(cam);
			set_detailed(cam);
			if (_detailed)
				show_detailed();
		}
	} // namespace Visualizer
} // namespace OoOVisual