// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementWire.h>
#include <Core/Commit/ReorderBuffer.h>
#include <Core/Fetch/FetchElements.h>
#include <Core/Fetch/Fetch.h>
#include <Core/Execution/ExecutionStage.h>
#include <cmath>
#include <imgui_internal.h>
namespace OoOVisual
{
	namespace Visualizer
	{


		static void pulser(Draw_Element_Wire& wire) {
			constexpr ImColor color_default{ 255, 255, 255, 255 };
			constexpr ImColor color_success{ 50, 255, 130, 255 };
			constexpr ImColor color_error{ 255, 80, 80, 255 };
			constexpr ImColor color_busy{ 255, 200, 50, 255 };
			constexpr ImColor color_flush{ 200, 100, 255, 255 };

			auto id{ wire.id() };

			switch (id) {
			case DRAW_ELEMENT_ID::WIRE_FETCHGROUP_TO_DISPATCHER:
			case DRAW_ELEMENT_ID::WIRE_FETCH_TO_FETCHGROUP:
				for (const auto& element : Core::Fetch_Group::group) {
					if (element.instruction != nullptr) {
						wire.trigger_pulse(color_success);
						break;
					}
				}
				break;

			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_FANOUT_VERTICAL:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_FANOUT_HORIZONTAL:
				for (const auto& feedback : Core::Dispatcher::last_dispatch_feedback()) {
					if (feedback == Core::DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH) {
						wire.trigger_pulse(color_busy);
						break;
					}
				}
				break;

			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_0:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_1:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_2:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_3:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_4:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_5:
			case DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_6:
			{
				auto feedback{ Core::Dispatcher::dispatch_feedback_of(static_cast<Core::RESERVATION_STATION_ID>((u8)id - (u8)DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_0)) };
				switch (feedback) {
				case Core::DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH:
					wire.trigger_pulse(color_success);
					break;
				case Core::DISPATCH_FEEDBACK::RESERVATION_STATION_WAS_FULL:
				case Core::DISPATCH_FEEDBACK::REGISTER_FILE_WAS_FULL:
				case Core::DISPATCH_FEEDBACK::REORDER_BUFFER_WAS_FULL:
					wire.trigger_pulse(color_error);
					break;
				case Core::DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH:
					break;
				default:
					break;
				}
			}
			break;

			case DRAW_ELEMENT_ID::WIRE_STATION_0_TO_ADDER:
			case DRAW_ELEMENT_ID::WIRE_STATION_1_TO_BITWISE:
			case DRAW_ELEMENT_ID::WIRE_STATION_2_TO_LESSTHAN:
			case DRAW_ELEMENT_ID::WIRE_STATION_3_TO_MULTIPLIER:
			case DRAW_ELEMENT_ID::WIRE_STATION_4_TO_DIVIDER:
			case DRAW_ELEMENT_ID::WIRE_STATION_5_TO_LOADSTORE:
			case DRAW_ELEMENT_ID::WIRE_STATION_6_TO_BRANCH:
			{
				auto idx{ u32(id) - u32(DRAW_ELEMENT_ID::WIRE_STATION_0_TO_ADDER) };
				if (Core::Execution_Stage::issued()[idx] != nullptr)
					wire.trigger_pulse(color_default);
				break;
			}

			case DRAW_ELEMENT_ID::WIRE_ADDER_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_BITWISE_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_LESSTHAN_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_MULTIPLIER_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_DIVIDER_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_LOADSTORE_TO_ROB:
			case DRAW_ELEMENT_ID::WIRE_BRANCH_TO_ROB:
			{
				auto idx{ u32(id) - u32(DRAW_ELEMENT_ID::WIRE_ADDER_TO_ROB) };
				if (Core::Execution_Stage::issued()[idx] != nullptr) {
					wire.trigger_pulse(color_success);
				}
				break;
			}

			case DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_HORIZONTAL:
			case DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_VERTICAL:
			case DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_TO_SIDE:
				if (Core::Reorder_Buffer::flushed())
					wire.trigger_pulse(color_flush);
				else if (Core::Reorder_Buffer::head_moved())
					wire.trigger_pulse(color_success);
				break;

			default:
				break;
			}
		}
		void Draw_Element_Wire::draw(const Camera& cam) {
			pulser(*this);
			ImDrawList* draw_list{ ImGui::GetWindowDrawList() };
			ImVec2 s{ cam.world_to_screen(_start) };
			ImVec2 e{ cam.world_to_screen(_end) };

			float current_time{ static_cast<float>(ImGui::GetTime()) };
			float time_since_pulse{ current_time - _last_pulse_time };
			float pulse_duration{ 0.8f };

			draw_list->AddLine(s, e, ImColor(0.15f, 0.15f, 0.15f, 1.0f), 3.0f);

			if (time_since_pulse < pulse_duration) {
				float intensity{ 1.0f - (time_since_pulse / pulse_duration) };
				float shimmer{ (sinf(current_time * 20.0f) + 1.0f) * 0.5f };

				ImColor final_color{ _pulse_color };
				final_color.Value.w = intensity * (0.4f + 0.6f * shimmer);

				draw_list->AddLine(s, e, final_color, 4.0f + (intensity * 4.0f));

				draw_list->AddLine(s, e, ImColor(1.0f, 1.0f, 1.0f, intensity * 0.8f), 1.5f);
			}
		}
		void Draw_Element_Wire::show_tooltip() const {
			ImGui::SetTooltip("This is a wire!");
		}

	}
}