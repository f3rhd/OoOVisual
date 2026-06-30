/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/Constants/Constants.h>
#include <Core/Execution/ExecutionResult.h>
#include <utility>
namespace OoOVisual
{
	namespace Core
	{

		class Execution_Unit_Adder { // does subtracting too.
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);
		};
		class Execution_Unit_Bitwise {
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);
		};

		// in real hardware designs having a seperate unit just for this bitch ass instruction is very nooby thing to do  but we gon do it in software anyway 
		class Execution_Unit_Set_Less_Than {
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);

		};
		class Execution_Unit_Multiplier {
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);
		};
		class Execution_Unit_Divider {
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);
		};

		class Execution_Unit_Load_Store {
		public:
			struct Buffer_Entry {
				// This field is needed for flushing upon misprediction
				u32			  timestamp{};
				// load uses this to write  the value to the register file
				reg_id_t      register_id{ Constants::INVALID_PHYSICAL_REGISTER_ID };
				// load  uses this to set the rob entry free
				size_t	      reorder_buffer_entry_index{ Constants::REORDER_BUFFER_SIZE + 1 };
				// this is  used by store  
				data_t        register_data{};
				// this is also both used by store and load
				memory_addr_t calculated_address{};
				// load uses this to broadcast the tag to the common data bus for forwarding logic
				u32			  producer_tag{ Constants::NO_PRODUCER_TAG };
				EXECUTION_UNIT_MODE mode{ EXECUTION_UNIT_MODE::UNKNOWN };
				/* load_buffer_entry : last store id*/
				u32 store_id{};
				/* instruction address is used to recover from load misspeculation */
				memory_addr_t  instruction_address = 0;
				Buffer_Entry() = default;
				Buffer_Entry(
					EXECUTION_UNIT_MODE mode_,
					u32 id_,
					u32 id__,
					size_t index,
					data_t data,
					memory_addr_t addr,
					u32 producer_tag_,
					memory_addr_t instr_addr,
					u32  store_id_ = Constants::NOT_STORE
				) : mode(mode_),
					timestamp(id_),
					register_id(id__),
					reorder_buffer_entry_index(index),
					register_data(data),
					calculated_address(addr),
					producer_tag(producer_tag_),
					instruction_address(instr_addr),
					store_id(store_id_) {}
			};
			static Execution_Result					address_generation_phase(Reservation_Station_Entry* source_entry);
			static Execution_Result					execute_load();
			static void								execute_store(u64 head);
			static bool								store_buffer_is_full() { return _store_buffer.size() >= Constants::STORE_BUFFER_SIZE; }
			static bool								load_buffer_is_full() { return _load_buffer.size() >= Constants::LOAD_BUFFER_SIZE; }
			static const std::vector<Buffer_Entry>& store_buffer() { return _store_buffer; } // used by visualizer
			static const std::vector<Buffer_Entry>& load_buffer() { return _load_buffer; } // used by visualizer
			static void							    reset();
			static Buffer_Entry* allocate_store_buffer_entry();
			static Buffer_Entry* allocate_load_buffer_entry();
			static time_t							flush_mispredicted(time_t timestamp);
			static void 						    remove_speculated_load(u64 reorder_buffer_entry_index);
			static Execution_Result 				resolve_speculated_loads(u64 head_that_points_to_store);
		private:
			// .first stands for executable load if it can be executed by forwarding .second will hold the store buffer entry index that is being forwarded from
			static std::pair<size_t, size_t>		find_load_that_is_executable();
		private:
			static std::vector<Buffer_Entry> 	_store_buffer;
			static std::vector<Buffer_Entry> 	_load_buffer;
			static std::vector<Buffer_Entry>	_speculative_load_buffer;
		};
		class Execution_Unit_Branch {
		public:
			static Execution_Result execute(const Reservation_Station_Entry* source_entry);
		};
	} // namespace Core
} // namespace OooVis