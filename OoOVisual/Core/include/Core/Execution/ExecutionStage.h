/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <vector>
#include <Core/Execution/ExecutionResult.h>
#include <Core/ReservationStation/ReservationStation.h>
namespace OoOVisual
{
	namespace Core
	{
		struct Execution_Stage {
			static void issue_and_execute(); // forwarding logic is implemented here along with execution
			static void reset();
			static const std::vector<const Reservation_Station_Entry*> issued(); // used by visualizer
		private:
			static std::vector<const Reservation_Station_Entry*> _last_issued_entries;
		};
	}
}