// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Execution/ExecutionStage.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Constants/Constants.h>
#include <Core/Commit/ReorderBuffer.h>
#include <Core/Fetch/FetchElements.h>
#include <Core/Fetch/Fetch.h>

namespace OoOVisual
{
	namespace Core
	{
		std::vector<const Reservation_Station_Entry*> Execution_Stage::_last_issued_entries(Constants::RESERVATION_STATION_AMOUNT, nullptr);
		void Execution_Stage::issue_and_execute() {

			std::vector<const Reservation_Station_Entry*> issued_entries(Constants::RESERVATION_STATION_AMOUNT - 1);
			std::vector<Execution_Result> execution_results;
			auto branch_execution_input = Reservation_Station_Pool::get_reservation_station(static_cast<RESERVATION_STATION_ID>(6)).issue();
			// upon misprediction execution unit may flush the instructions that are ready to execute.
			execution_results.reserve(Constants::RESERVATION_STATION_AMOUNT + 1);
			auto branch_execution_result{ execution_results.emplace_back(Execution_Unit_Branch::execute(branch_execution_input)) };
			for (size_t i{}; i < Constants::RESERVATION_STATION_AMOUNT - 1; i++) {
				issued_entries[i] = Reservation_Station_Pool::get_reservation_station(static_cast<RESERVATION_STATION_ID>(i)).issue();
			}
			execution_results.emplace_back(Execution_Unit_Load_Store::execute_load());
			execution_results.emplace_back(Execution_Unit_Load_Store::address_generation_phase(const_cast<Reservation_Station_Entry*>(issued_entries[5])));

			execution_results.emplace_back(Execution_Unit_Adder::execute(issued_entries[0]));
			execution_results.emplace_back(Execution_Unit_Bitwise::execute(issued_entries[1]));
			execution_results.emplace_back(Execution_Unit_Set_Less_Than::execute(issued_entries[2]));
			execution_results.emplace_back(Execution_Unit_Multiplier::execute(issued_entries[3]));
			execution_results.emplace_back(Execution_Unit_Divider::execute(issued_entries[4]));

			/* upon misprediciton stall the frontend pipeline and drain the OoO core
			   this is needed because we need to recover the register alias table as soon as possible
			   there could be a possibility where once a misprediction is detected the branch instruction is not
			   at the head of the reorder buffer which means it will take multiple cycles to restore the alias table
			   which may cause incoming instructions to use a wrong alias table
			*/
			if (branch_execution_result.misprediction_was_detected) {
				Fetch_Unit::stall();
			}
			// forward the data to reservation stations
			for (const auto& data : execution_results) {

				if (data.kind & Constants::EXECUTION_RESULT_FORWARD_ONLY) {
					Reservation_Station_Pool::wakeup(data.producer_tag, data.produced_data);
				}
				else if (data.kind & Constants::EXECUTION_RESULT_STATION_DEALLOCATE_ONLY) {
					Reservation_Station_Pool::deallocate_entry(data.producer_tag);
				}
				else if (data.kind & Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD) {
					Reservation_Station_Pool::wakeup(data.producer_tag, data.produced_data);
					Reservation_Station_Pool::deallocate_entry(data.producer_tag);
				}
			}
			// visualizer will use these to render stuff
			_last_issued_entries = issued_entries;
			_last_issued_entries.emplace_back(branch_execution_input);
		}

		void Execution_Stage::reset() {
			_last_issued_entries = std::vector<const Reservation_Station_Entry*>(Constants::RESERVATION_STATION_AMOUNT, nullptr);
		}

		const std::vector<const OoOVisual::Core::Reservation_Station_Entry*> Execution_Stage::issued() {
			return _last_issued_entries;
		}

	} // namespace Core
} // namespace OoOVis

