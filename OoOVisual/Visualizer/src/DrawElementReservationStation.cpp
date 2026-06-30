// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include "../../vendor/ImGui/imgui.h"
#include <Core/Constants/Constants.h>
#include <Core/Execution/ExecutionUnitModes.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Types/Types.h>
#include <Visualizer/Camera.h>
#include <Visualizer/DrawElement.h>
#include <Visualizer/DrawElementCoreUnit.h>
#include <Visualizer/DrawElementReservationStation.h>

namespace OoOVisual
{
	namespace Visualizer
	{


		Draw_Element_Reservation_Station::Draw_Element_Reservation_Station(DRAW_ELEMENT_ID id, const ImVec2& position, const ImVec2& dimension) : Draw_Element_Core_Unit(id, position, dimension) {
			switch (_id) {
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_0:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::ADD_SUB);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_1:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::BITWISE);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_2:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::SET_LESS);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_3:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::MULTIPLIER);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_4:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::DIVIDER);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_5:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::LOAD_STORE);
				break;
			case OoOVisual::Visualizer::DRAW_ELEMENT_ID::STATION_6:
				_core_station = &Core::Reservation_Station_Pool::get_reservation_station(Core::RESERVATION_STATION_ID::BRANCH);
				break;
			default:
				break;
			}
		}

		void Draw_Element_Reservation_Station::show_tooltip() const {
			ImGui::BeginTooltip();
			ImGui::Text("reservation station entries");
			ImGui::Separator();

			if (ImGui::BeginTable("##ReservationStation", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
				ImGui::TableSetupColumn("slot");
				ImGui::TableSetupColumn("time");
				ImGui::TableSetupColumn("busy");
				ImGui::TableSetupColumn("ready");
				ImGui::TableSetupColumn("dest");
				ImGui::TableSetupColumn("src1/t1");
				ImGui::TableSetupColumn("src2/t2");
				ImGui::TableSetupColumn("special");
				ImGui::TableSetupColumn("rob idx");
				ImGui::TableHeadersRow();

				for (u32 i{ 0 }; i < Core::Constants::RESERVATION_STATION_SIZE; ++i) {
					const auto& entry{ _core_station->getc()[i] };

					// skip entries that aren't being used
					if (entry.mode == Core::EXECUTION_UNIT_MODE::UNKNOWN) {
						continue;
					}

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%u", i);

					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 1.0f, 1.0f }, "%u", entry.timestamp);

					ImGui::TableSetColumnIndex(2);
					if (entry.busy) {
						ImGui::TextColored(ImVec4{ 1.0f, 0.4f, 0.4f, 1.0f }, "busy");
					}
					else {
						ImGui::TextDisabled("idle");
					}

					ImGui::TableSetColumnIndex(3);
					if (entry.ready) {
						ImGui::TextColored(ImVec4{ 0.2f, 1.0f, 0.2f, 1.0f }, "ready");
					}
					else {
						ImGui::TextColored(ImVec4{ 1.0f, 0.8f, 0.2f, 1.0f }, "wait");
					}

					ImGui::TableSetColumnIndex(4);
					if (entry.destination_register_id != Core::Constants::INVALID_PHYSICAL_REGISTER_ID) {
						ImGui::Text("p%u", entry.destination_register_id);
					}
					else {
						ImGui::TextDisabled("-");
					}

					ImGui::TableSetColumnIndex(5);
					if (entry.producer_tag1 != Core::Constants::NO_PRODUCER_TAG) {
						ImGui::TextColored(ImVec4{ 1.0f, 0.6f, 0.6f, 1.0f }, "t:%u", entry.producer_tag1);
					}
					else {
						ImGui::Text("v:%d", entry.src1.SIGNED);
					}

					ImGui::TableSetColumnIndex(6);
					if (entry.producer_tag2 != Core::Constants::NO_PRODUCER_TAG) {
						ImGui::TextColored(ImVec4{ 1.0f, 0.6f, 0.6f, 1.0f }, "t:%u", entry.producer_tag2);
					}
					else {
						ImGui::Text("v:%d", entry.src2.SIGNED);
					}

					ImGui::TableSetColumnIndex(7);
					if (_core_station->id() == Core::RESERVATION_STATION_ID::BRANCH) {
						ImGui::Text("tgt: 0x%08X", entry.branch_target);
						if (entry.fetch_unit_prediction != Core::Constants::NOT_BRANCH_INSTRUCTION) {
							ImGui::SameLine();
							ImGui::TextColored(ImVec4{ 0.4f, 0.8f, 1.0f, 1.0f }, "[pred]");
						}
					}
					else if (_core_station->id() == Core::RESERVATION_STATION_ID::LOAD_STORE) {
						if (entry.destination_register_id_as_ofsset) {
							ImGui::Text("store");
						}
						else {
							ImGui::Text("load");
						}
					}
					else {
						ImGui::TextDisabled("std reg");
					}

					ImGui::TableSetColumnIndex(8);
					ImGui::Text("#%lu", entry.reorder_buffer_entry_index);
					ImGui::TableSetColumnIndex(9);
					ImGui::TextColored(ImVec4{ 1.0f, 0.5f, 0.0f, 1.0f }, "tag:%u", entry.self_tag);
				}
				ImGui::EndTable();
			}
			ImGui::EndTooltip();
		}
		void Draw_Element_Reservation_Station::draw(const Camera& cam) {

			show_architectural(cam);
			if (is_hovered(cam))
				show_tooltip();
		}
	}
}