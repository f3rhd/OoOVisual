/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <array>
#include <Core/Constants/Constants.h>
#include <Core/Fetch/FetchElements.h>
#include <Core/ReservationStation/ReservationStation.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/Types/Types.h>
#include <Frontend/Parser/Instruction.h>
#include <map>
namespace OoOVisual
{
	namespace Core
	{
		enum class DISPATCH_FEEDBACK : u8 {
			NO_INSTRUCTION_TO_DISPATCH,
			RESERVATION_STATION_WAS_FULL,
			STORE_BUFFER_WAS_FULL,
			LOAD_BUFFER_WAS_FULL,
			REGISTER_FILE_WAS_FULL,
			REORDER_BUFFER_WAS_FULL,
			SUCCESSFUL_DISPATCH
		};
		class Dispatcher {

		public:
			Dispatcher() = default;
			static std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH>                                      dispatch_fetch_group();
			static const std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH>& last_dispatch_feedback();
			static DISPATCH_FEEDBACK																		dispatch_feedback_of(RESERVATION_STATION_ID id); // used by visualizer
			static void																						reset();
		private:
			static DISPATCH_FEEDBACK																		dispatch_fetch_element(const Fetch_Element& fetch_element);
			static DISPATCH_FEEDBACK																		dispatch_register_instruction(const Fetch_Element& element, Reservation_Station& station);
			static DISPATCH_FEEDBACK																		dispatch_load_instruction(const Fetch_Element& element, Reservation_Station& station);
			static DISPATCH_FEEDBACK																		dispatch_store_instruction(const Fetch_Element& element, Reservation_Station& station);
			static DISPATCH_FEEDBACK																		dispatch_branch_instruction(const Fetch_Element& element, Reservation_Station& station);
			static DISPATCH_FEEDBACK																		dispatch_jump_instruction(const Fetch_Element& element, Reservation_Station& station);
			static RESERVATION_STATION_ID																	get_register_instruction_reservation_station_id(FrontEnd::Instruction* instruction);
		private:
			static std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH>                                       _last_dispatch_feedback;
			static std::map<Core::RESERVATION_STATION_ID, DISPATCH_FEEDBACK>									_station_dispatch_map; //used by visualizer
			static u32																							_last_store_id;

		};

	} // namespace Core

} // namespace OoOVis
