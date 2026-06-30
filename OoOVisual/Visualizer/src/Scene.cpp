// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/Scene.h>
#include <Visualizer/DrawElements.h>
#include <Visualizer/Constants.h>
#include <Core/Core.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		void Scene::init() {
			float screen_width{ 1400 };
			float center_x{ screen_width / 2.0f };

			// top section: frontend pipeline (vertical spine) 

			//  fetch_unit (top of vertical pipeline)
			ImVec2 fetch_pos{ center_x - 140.0f, 40.0f };
			ImVec2 fetch_size{ 280.0f, 70.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Fetch_Unit>(
				Visualizer::DRAW_ELEMENT_ID::FETCH_UNIT, fetch_pos, fetch_size
			));

			// fetch group 
			ImVec2 fetch_group_pos{ center_x - 140.0f, 150.0f };
			ImVec2 fetch_group_size{ 280.0f, 70.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Fetch_Group>(
				Visualizer::DRAW_ELEMENT_ID::FETCH_GROUP, fetch_group_pos, fetch_group_size
			));

			// wire: fetch_unit -> fetch group (vertical)
			ImVec2 fetch_bottom_center{ fetch_pos.x + (fetch_size.x / 2.0f), fetch_pos.y + fetch_size.y };
			ImVec2 fetch_group_top_center{ fetch_group_pos.x + (fetch_group_size.x / 2.0f), fetch_group_pos.y };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_FETCH_TO_FETCHGROUP,
				fetch_bottom_center,
				fetch_group_top_center
			));

			// dispatcher 
			ImVec2 dispatch_pos{ center_x - 140.0f, 270.0f };
			ImVec2 dispatch_size{ 280.0f, 80.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Dispatcher>(
				Visualizer::DRAW_ELEMENT_ID::DISPATCHER, dispatch_pos, dispatch_size
			));

			// vertical wire: fetch group -> dispatcher 
			ImVec2 fetch_group_bottom_center{ fetch_group_pos.x + (fetch_group_size.x / 2.0f), fetch_group_pos.y + fetch_group_size.y };
			ImVec2 dispatch_top_center{ dispatch_pos.x + (dispatch_size.x / 2.0f), dispatch_pos.y };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_FETCHGROUP_TO_DISPATCHER,
				fetch_group_bottom_center,
				dispatch_top_center
			));

			// right side: physical register file 
			ImVec2 reg_file_pos{ 1050.0f, 150.0f };
			ImVec2 reg_file_size{ 280.0f, 140.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Register_Manager>(
				Visualizer::DRAW_ELEMENT_ID::REGISTER_MANAGER, reg_file_pos, reg_file_size
			));

			// middle section: reservation stations  
			float stations_start_y{ 420.0f };
			ImVec2 station_size{ 140.0f, 70.0f };

			float station_spacing{ 35.0f };
			float total_stations_width{ (7 * station_size.x) + (6 * station_spacing) };
			float stations_start_x{ (screen_width - total_stations_width) / 2.0f };

			std::vector<ImVec2> station_positions{};
			std::vector<ImVec2> station_bottom_centers{};
			std::vector<ImVec2> station_top_centers{};

			for (int i{ 0 }; i < 7; i++) {
				float station_x{ stations_start_x + (i * (station_size.x + station_spacing)) };
				ImVec2 station_pos{ station_x, stations_start_y };
				station_positions.emplace_back(station_pos);

				ImVec2 top_center{ station_pos.x + (station_size.x / 2.0f), station_pos.y };
				ImVec2 bottom_center{ station_pos.x + (station_size.x / 2.0f), station_pos.y + station_size.y };
				station_top_centers.emplace_back(top_center);
				station_bottom_centers.emplace_back(bottom_center);

				Visualizer::DRAW_ELEMENT_ID station_id{ static_cast<Visualizer::DRAW_ELEMENT_ID>(
					static_cast<int>(Visualizer::DRAW_ELEMENT_ID::STATION_0) + i
				) };

				_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Reservation_Station>(
					station_id, station_pos, station_size
				));
			}

			// dispatcher -> stations wiring 
			ImVec2 dispatch_bottom_center{ dispatch_pos.x + (dispatch_size.x / 2.0f), dispatch_pos.y + dispatch_size.y };
			float dispatch_fanout_y{ dispatch_bottom_center.y + 30.0f };

			// wire: main vertical drop from dispatcher to fanout level
			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_FANOUT_VERTICAL,
				dispatch_bottom_center,
				ImVec2{ dispatch_bottom_center.x, dispatch_fanout_y }
			));

			// wire: horizontal fanout line spanning all stations
			float fanout_left{ station_top_centers[0].x };
			float fanout_right{ station_top_centers[6].x };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_FANOUT_HORIZONTAL,
				ImVec2{ fanout_left, dispatch_fanout_y },
				ImVec2{ fanout_right, dispatch_fanout_y }
			));

			// wires: vertical drops from fanout line to each station
			Visualizer::DRAW_ELEMENT_ID dispatch_to_station_wire_ids[7]{
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_0,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_1,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_2,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_3,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_4,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_5,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DISPATCHER_TO_STATION_6
			};

			for (int i{ 0 }; i < 7; i++) {
				_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
					dispatch_to_station_wire_ids[i],
					ImVec2{ station_top_centers[i].x, dispatch_fanout_y },
					station_top_centers[i]
				));
			}

			// execution units row 
			float exec_units_y{ stations_start_y + station_size.y + 80.0f };
			ImVec2 exec_unit_size{ 140.0f, 70.0f };

			Visualizer::DRAW_ELEMENT_ID exec_unit_ids[7]{
				Visualizer::DRAW_ELEMENT_ID::ADDER_SUBTRACTOR_UNIT,
				Visualizer::DRAW_ELEMENT_ID::BITWISE_UNIT,
				Visualizer::DRAW_ELEMENT_ID::LESS_THAN_SETTER_UNIT,
				Visualizer::DRAW_ELEMENT_ID::MULTIPLIER_UNIT,
				Visualizer::DRAW_ELEMENT_ID::DIVIDER_UNIT,
				Visualizer::DRAW_ELEMENT_ID::LOAD_STORE_UNIT,
				Visualizer::DRAW_ELEMENT_ID::BRANCH_UNIT
			};

			Visualizer::DRAW_ELEMENT_ID station_to_exec_wire_ids[7]{
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_0_TO_ADDER,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_1_TO_BITWISE,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_2_TO_LESSTHAN,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_3_TO_MULTIPLIER,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_4_TO_DIVIDER,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_5_TO_LOADSTORE,
				Visualizer::DRAW_ELEMENT_ID::WIRE_STATION_6_TO_BRANCH
			};

			std::vector<ImVec2> exec_unit_positions{};
			std::vector<ImVec2> exec_unit_bottom_centers{};

			// create execution units and wires from stations
			for (int i{ 0 }; i < 7; i++) {
				ImVec2 exec_unit_pos{ station_positions[i].x, exec_units_y };
				exec_unit_positions.emplace_back(exec_unit_pos);

				ImVec2 bottom_center{ exec_unit_pos.x + (exec_unit_size.x / 2.0f), exec_unit_pos.y + exec_unit_size.y };
				exec_unit_bottom_centers.emplace_back(bottom_center);

				_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Execution_Unit>(
					exec_unit_ids[i], exec_unit_pos, exec_unit_size
				));

				// wire: station -> execution unit (vertical)
				ImVec2 exec_unit_top_center{ exec_unit_pos.x + (exec_unit_size.x / 2.0f), exec_unit_pos.y };

				_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
					station_to_exec_wire_ids[i],
					station_bottom_centers[i],
					exec_unit_top_center
				));
			}

			// bottom section: reorder buffer 
			float rob_y{ exec_units_y + exec_unit_size.y + 90.0f };
			ImVec2 rob_pos{ 80.0f, rob_y };
			ImVec2 rob_size{ screen_width - 160.0f, 100.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Reorder_Buffer>(
				Visualizer::DRAW_ELEMENT_ID::REORDER_BUFFER, rob_pos, rob_size
			));

			// execution units -> rob wiring (individual connections, no cdb backbone) 
			Visualizer::DRAW_ELEMENT_ID exec_to_rob_wire_ids[7]{
				Visualizer::DRAW_ELEMENT_ID::WIRE_ADDER_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_BITWISE_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_LESSTHAN_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_MULTIPLIER_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_DIVIDER_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_LOADSTORE_TO_ROB,
				Visualizer::DRAW_ELEMENT_ID::WIRE_BRANCH_TO_ROB
			};

			// direct vertical connections from each execution unit to rob top
			for (int i{ 0 }; i < 7; i++) {
				ImVec2 exec_bottom{ exec_unit_bottom_centers[i] };
				ImVec2 rob_top_at_exec_x{ exec_bottom.x, rob_pos.y };

				_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
					exec_to_rob_wire_ids[i],
					exec_bottom,
					rob_top_at_exec_x
				));
			}

			// feedback path: rob -> physical register file 
			// manhattan routing: right -> up -> left 
			ImVec2 rob_right_center{ rob_pos.x + rob_size.x, rob_pos.y + (rob_size.y / 2.0f) };
			ImVec2 reg_file_right_center{ reg_file_pos.x + reg_file_size.x, reg_file_pos.y + (reg_file_size.y / 2.0f) };

			float rob_to_reg_corner_x{ rob_right_center.x + 40.0f };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_HORIZONTAL,
				rob_right_center,
				ImVec2{ rob_to_reg_corner_x, rob_right_center.y }
			));

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_VERTICAL,
				ImVec2{ rob_to_reg_corner_x, rob_right_center.y },
				ImVec2{ rob_to_reg_corner_x, reg_file_right_center.y }
			));

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Wire>(
				Visualizer::DRAW_ELEMENT_ID::WIRE_ROB_TO_REGFILE_TO_SIDE,
				ImVec2{ rob_to_reg_corner_x, reg_file_right_center.y },
				reg_file_right_center
			));
			ImVec2 screen_pos{ reg_file_pos.x + reg_file_size.x + 100, reg_file_pos.y };
			ImVec2 screen_size{ Core::Constants::CORE_SCREEN_WIDTH, Core::Constants::CORE_SCREEN_HEIGHT };

			_units.emplace_back(std::make_unique<Visualizer::Draw_Element_Screen>(
				Visualizer::DRAW_ELEMENT_ID::SCREEN,
				screen_pos,
				screen_size
			));
		}
		void Scene::play() {
			_scene_camera.update();
			ImGui::Begin("Pipeline Canvas", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
			ImGui::SetNextItemWidth(150.0f);
			ImGui::SliderFloat("Speed", Core::tick_speed(), 0.0f, 1.0f, "%.2f");
			if (ImGui::Button("Run")) {
				Core::set_core_mode(Core::CORE_MODE::RUN);
			}
			ImGui::SameLine();
			if (ImGui::Button("Step")) {
				Core::set_core_mode(Core::CORE_MODE::STEP);
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				Core::set_core_mode(Core::CORE_MODE::STOP);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				Core::reset();
			}
			for (const auto& element : _units) {
				element->draw(_scene_camera);
			}
			ImGui::End();
		}

		void Scene::clear() {
			_units.clear();
		}

	}
}