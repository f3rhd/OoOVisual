/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Core/Types/Types.h>
#include <Core/Constants/Constants.h>
#include <Core/Execution/ExecutionUnitModes.h>
namespace OoOVisual
{
	namespace Core
	{
		enum class RESERVATION_STATION_ID : u8 {
			ADD_SUB,
			BITWISE,
			SET_LESS,
			MULTIPLIER,
			DIVIDER,
			LOAD_STORE,
			BRANCH,
			UNKNOWN
		};
		struct Reservation_Station_Entry {
			/* elementary station entry fields */
			u64	   reorder_buffer_entry_index = 0;
			data_t src1 = {};
			data_t src2 = {};
			u32    producer_tag1 = Constants::NO_PRODUCER_TAG;
			u32    producer_tag2 = Constants::NO_PRODUCER_TAG;
			u32    self_tag = Constants::NO_PRODUCER_TAG;
			u32    instruction_address = 0;
			reg_id_t destination_register_id = Constants::INVALID_PHYSICAL_REGISTER_ID;
			/* branch target addres of branch instructions*/
			memory_addr_t branch_target = 0;
			/*timestamp given to any instruction when it was fetched */
			u32 timestamp = Constants::TIME_ZERO;
			/* for load instruction it implies the last store_id
			   for store instructin it implies the self_store_id
			*/
			u32 store_id = Constants::NOT_STORE;
			/* prediction made by fetch unit when it saw a branch instruction */
			u8 fetch_unit_prediction = Constants::NOT_BRANCH_INSTRUCTION;
			/* elementary station fields */
			bool   ready = false;
			bool   busy = false;
			/* boolean to indicate that the instruction uses destination register field as offset */
			bool   destination_register_id_as_ofsset = false;
			/* when a load address is generated  flag will turn true
				so we dont try to address generate twice
			*/
			bool generated_addrress_for_load = false;
			/* execution unit mode */
			EXECUTION_UNIT_MODE mode = EXECUTION_UNIT_MODE::UNKNOWN;
		};

	} // namespace Core
} // namespace OoOVis
