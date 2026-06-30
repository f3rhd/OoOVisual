// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Commit/ReorderBuffer.h>
#include <Core/Constants/Constants.h>
#include <Core/DCache/DCache.h>
#include <Core/Execution/ExecutionResult.h>
#include <Core/Execution/ExecutionUnitModes.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/Fetch/Fetch.h>
#include <Core/Fetch/FetchElements.h>
#include <Core/MMIO/ScreenMMIO.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Types/Types.h>
#include <cstdint>
#include <utility>
#include <vector>
#include <algorithm>
#ifdef DEBUG_PRINTS
#include <iostream>
#include <format>
#endif

namespace OoOVisual
{
	namespace Core
	{


		Execution_Result Execution_Unit_Adder::execute(const Reservation_Station_Entry* source_entry) {
			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			Execution_Result data{};
			data.kind = Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD;
			data.producer_tag = source_entry->self_tag;
			switch (source_entry->mode) {
			case EXECUTION_UNIT_MODE::ADD_SUB_UNIT_ADD:
				data.produced_data.SIGNED = source_entry->src1.SIGNED + source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::ADD_SUB_UNIT_SUB:
				data.produced_data.SIGNED = source_entry->src1.SIGNED - source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::ADD_SUB_UNIT_AUIPC: // immediate value lives in the source 2
				data.produced_data.UNSIGNED = (source_entry->src2.UNSIGNED << 12) & 0xFFFFF000 + Fetch_Unit::program_counter();
				break;
			case EXECUTION_UNIT_MODE::ADD_SUB_UNIT_LOAD_UPPER:
				data.produced_data.UNSIGNED = (source_entry->src2.UNSIGNED << 12) & 0xFFFFF000;
				break;
				// shouldnt happen
			default:
				break;
			}
#ifdef DEBUG_PRINTS
			std::cout << Constants::MAGENTA << std::format("Instructions[{}] timestamp:{} is executing.", source_entry->instruction_address, source_entry->timestamp);
#endif
			Register_Manager::write(source_entry->destination_register_id, data.produced_data);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			return data;
		}

		Execution_Result Execution_Unit_Bitwise::execute(const Reservation_Station_Entry* source_entry) {
			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			Execution_Result result{};
			result.kind = Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD;
			result.producer_tag = source_entry->self_tag;
			switch (source_entry->mode) {
			case EXECUTION_UNIT_MODE::BITWISE_AND:
				result.produced_data.SIGNED = source_entry->src1.SIGNED & source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BITWISE_XOR:
				result.produced_data.SIGNED = source_entry->src1.SIGNED | source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::LOGICAL_XOR:
				result.produced_data.SIGNED = source_entry->src1.SIGNED ^ source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BITWISE_SHIFT_LEFT_LOGICAL:
				result.produced_data.UNSIGNED = source_entry->src1.UNSIGNED << source_entry->src2.UNSIGNED;
				break;
			case EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_LOGICAL:
				result.produced_data.UNSIGNED = source_entry->src1.UNSIGNED >> source_entry->src2.UNSIGNED;
				break;
			case EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_ARITHMETIC:
				result.produced_data.SIGNED = source_entry->src1.SIGNED >> source_entry->src2.UNSIGNED;
				break;
				// shouldnt happen
			default:
				break;
			}

			Register_Manager::write(source_entry->destination_register_id, result.produced_data);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			return result;
		}

		Execution_Result Execution_Unit_Set_Less_Than::execute(const Reservation_Station_Entry* source_entry) {
			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			Execution_Result result{};
			result.kind = Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD;
			result.producer_tag = source_entry->self_tag;
			result.produced_data.SIGNED = source_entry->src1.SIGNED < source_entry->src2.SIGNED ? 1 : 0;
			Register_Manager::write(source_entry->destination_register_id, result.produced_data);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			return result;
		}

		Execution_Result Execution_Unit_Multiplier::execute(const Reservation_Station_Entry* source_entry) {
			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			Execution_Result result{};
			result.kind = Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD;
			result.producer_tag = source_entry->self_tag;
			switch (source_entry->mode) {
			case EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_SIGNED:
			case EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH:
				result.produced_data.SIGNED = source_entry->src1.SIGNED * source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_SIGNED_UNSIGNED:
			{
				// MULHSU: src1 is signed, src2 is unsigned
				// We cast to 64-bit to ensure we don't lose the overflow
				int64_t full_product(static_cast<int64_t>(source_entry->src1.SIGNED) * static_cast<uint64_t>(source_entry->src2.UNSIGNED));
				result.produced_data.SIGNED = static_cast<int32_t>(full_product >> 32);
				break;
			}
			case EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_UNSIGNED:
			{
				// MULHU: Both src1 and src2 are unsigned
				uint64_t full_product{ static_cast<uint64_t>(source_entry->src1.UNSIGNED) * static_cast<uint64_t>(source_entry->src2.UNSIGNED) };
				result.produced_data.UNSIGNED = static_cast<uint32_t>(full_product >> 32);
				break;
			}
			// shouldnt happen
			default:
				break;

			}
			Register_Manager::write(source_entry->destination_register_id, result.produced_data);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			return result;
		}

		Execution_Result Execution_Unit_Divider::execute(const Reservation_Station_Entry* source_entry) {
			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			Execution_Result result{};
			result.kind = Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD;
			result.producer_tag = source_entry->self_tag;
			switch (source_entry->mode) {
			case EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_SIGNED:
				result.produced_data.SIGNED = source_entry->src1.SIGNED / source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_UNSIGNED:
				result.produced_data.UNSIGNED = source_entry->src1.UNSIGNED / source_entry->src2.UNSIGNED;
				break;

			case EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_SIGNED:
				result.produced_data.SIGNED = source_entry->src1.SIGNED % source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_UNSIGNED:
				result.produced_data.UNSIGNED = source_entry->src1.UNSIGNED % source_entry->src2.UNSIGNED;
				break;
				// shouldnt happen
			default:
				break;
			}
			Register_Manager::write(source_entry->destination_register_id, result.produced_data);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			return result;
		}
		std::vector<Execution_Unit_Load_Store::Buffer_Entry> Execution_Unit_Load_Store::_store_buffer{};
		std::vector<Execution_Unit_Load_Store::Buffer_Entry> Execution_Unit_Load_Store::_load_buffer{};
		std::vector<Execution_Unit_Load_Store::Buffer_Entry> Execution_Unit_Load_Store::_speculative_load_buffer{};
		Execution_Result Execution_Unit_Load_Store::address_generation_phase(Reservation_Station_Entry* source_entry) {

			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			memory_addr_t address{}; ;
			data_t        register_data{}; ;
			if (source_entry->destination_register_id_as_ofsset) {
				address = source_entry->src1.SIGNED + static_cast<offset_t>(source_entry->destination_register_id);
				register_data = source_entry->src2;
				/* if a store instruction is issued to a reservation station an entry in store buffer is allocated for it during dispatch stage*/
				auto store_it{
					std::find_if(
						_store_buffer.begin(),
						_store_buffer.end(),
						[&](const Buffer_Entry& entry) {
							return entry.reorder_buffer_entry_index == source_entry->reorder_buffer_entry_index;
						}
					)
				};
				store_it->mode = source_entry->mode;
				store_it->timestamp = source_entry->timestamp;
				store_it->register_id = Constants::INVALID_PHYSICAL_REGISTER_ID;
				store_it->register_data = register_data;
				store_it->calculated_address = address;
				store_it->producer_tag = source_entry->self_tag;
				store_it->instruction_address = source_entry->instruction_address;
				store_it->store_id = source_entry->store_id;
#ifdef DEBUG_PRINTS
				std::cout << std::format("Created entry in the store buffer: timestamp:{}, address:{}\n", source_entry->timestamp, address);
#endif
				// tell the rob that address and the data is ready
				Reorder_Buffer::set_ready(static_cast<u32>(source_entry->reorder_buffer_entry_index));
				return { Constants::EXECUTION_RESULT_STATION_DEALLOCATE_ONLY,0,source_entry->self_tag };
			}
			if (source_entry->generated_addrress_for_load)
				return { Constants::EXECUTION_RESULT_INVALID };
			address = source_entry->src1.SIGNED + source_entry->src2.SIGNED;
			auto load_it{
				std::find_if(
					_load_buffer.begin(),
					_load_buffer.end(),
					[&](const Buffer_Entry& entry) {
						return entry.reorder_buffer_entry_index == source_entry->reorder_buffer_entry_index;
					}
				)
			};
			load_it->mode = source_entry->mode;
			load_it->timestamp = source_entry->timestamp;
			load_it->register_id = source_entry->destination_register_id;
			load_it->register_data = register_data;
			load_it->calculated_address = address;
			load_it->producer_tag = source_entry->self_tag;
			load_it->instruction_address = source_entry->instruction_address;
			load_it->store_id = source_entry->store_id;
			source_entry->generated_addrress_for_load = true;
#ifdef DEBUG_PRINTS
			std::cout << std::format("Created entry in the load buffer buffer: timestamp:{}, destination_reg:{}, address:{}\n", source_entry->timestamp, source_entry->destination_register_id, address);
#endif
			return { Constants::EXECUTION_RESULT_INVALID,0,source_entry->self_tag };
		}

		std::pair<size_t, size_t> Execution_Unit_Load_Store::find_load_that_is_executable() {
			if (
				std::all_of(
					_load_buffer.begin(),
					_load_buffer.end(),
					[](const Buffer_Entry& entry) {
						return entry.mode == EXECUTION_UNIT_MODE::UNKNOWN;
					}
				)
			) {
				return { Constants::EXECUTABLE_LOAD_DOES_NOT_EXIST,Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE };
			}

			// maybe store buffer is empty?
			if (
				std::all_of(
					_store_buffer.begin(),
					_store_buffer.end(),
					[](const Buffer_Entry& entry) {
						return entry.mode == EXECUTION_UNIT_MODE::UNKNOWN;
					}
				)
			) {
				auto it{
					std::find_if(
						_load_buffer.begin(),
						_load_buffer.end(),
						[](const Buffer_Entry& entry) {
							return entry.mode != EXECUTION_UNIT_MODE::UNKNOWN;
						}
					)
				};
				return { it - _load_buffer.begin(),Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE };
			}
			// maybe we can forward anything?
			size_t store_buffer_entry_index_that_is_forwarded_from{ Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE };

			// forward from the latest store instruction that writes to the same address
			u32 max_store_id{};
			for (size_t i{}; i < _load_buffer.size(); i++) {
				const auto& load_buffer_entry = _load_buffer.at(i);
				if (load_buffer_entry.mode == EXECUTION_UNIT_MODE::UNKNOWN) continue; // address is not generated yet
				for (size_t j{}; j < _store_buffer.size(); j++) {
					const auto& store_buffer_entry = _store_buffer.at(j);
					if (store_buffer_entry.mode == EXECUTION_UNIT_MODE::UNKNOWN) continue; // address is not generated yet
					// we dont care about the stores that came after us 
					if (store_buffer_entry.store_id > load_buffer_entry.store_id) {
						continue;
					}
					if (store_buffer_entry.calculated_address == load_buffer_entry.calculated_address &&
						store_buffer_entry.store_id >= max_store_id
					) {
						store_buffer_entry_index_that_is_forwarded_from = j;
						max_store_id = store_buffer_entry.store_id;
					}
				}
				if (store_buffer_entry_index_that_is_forwarded_from != Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE) {
					return { i,store_buffer_entry_index_that_is_forwarded_from };
				}
			}

			// maybe we can bypass?
			size_t bypassable_load_entry_index{};
			bool can_bypass{ false };
			for (size_t i{ 0 }; i < _load_buffer.size(); i++) {
				const Buffer_Entry& load_entry = _load_buffer.at(i);
				if (load_entry.mode == EXECUTION_UNIT_MODE::UNKNOWN) continue; // means that address for this is not generated yet
				for (const auto& store_entry : _store_buffer) {
					if (load_entry.calculated_address != store_entry.calculated_address || store_entry.mode == EXECUTION_UNIT_MODE::UNKNOWN)
						can_bypass = true;
					else
						can_bypass = false;
				}
				if (can_bypass) {
					bypassable_load_entry_index = i;
					break;
				}

			}
			if (can_bypass)
				return { bypassable_load_entry_index,Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE };

			return { Constants::EXECUTABLE_LOAD_DOES_NOT_EXIST,Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE };
		}

		static u8 access_size_from_mode(EXECUTION_UNIT_MODE mode) {
			switch (mode) {
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_WORD:
			case EXECUTION_UNIT_MODE::LOAD_STORE_STORE_WORD:              return 4;
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF:
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF_UNSIGNED:
			case EXECUTION_UNIT_MODE::LOAD_STORE_STORE_HALF:              return 2;
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE:
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE_UNSIGNED:
			case EXECUTION_UNIT_MODE::LOAD_STORE_STORE_BYTE:              return 1;
			default:                                 return 0;
			}
		}

		enum class STORE_FORWARD_RESULT { FULL_FORWARD, PARTIAL_OVERLAP, NO_OVERLAP };

		static STORE_FORWARD_RESULT classify_overlap(const Execution_Unit_Load_Store::Buffer_Entry& load, const Execution_Unit_Load_Store::Buffer_Entry& store) {
			memory_addr_t l_start{ load.calculated_address };
			memory_addr_t l_end{ l_start + access_size_from_mode(load.mode) };
			memory_addr_t s_start{ store.calculated_address };
			memory_addr_t s_end{ s_start + access_size_from_mode(store.mode) };

			if (l_end <= s_start || s_end <= l_start)
				return STORE_FORWARD_RESULT::NO_OVERLAP;

			if (s_start <= l_start && l_end <= s_end)
				return STORE_FORWARD_RESULT::FULL_FORWARD;

			return STORE_FORWARD_RESULT::PARTIAL_OVERLAP;
		}
		static bool mode_is_signed(EXECUTION_UNIT_MODE mode) {
			switch (mode) {
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE:
			case EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF:  return true;
			default:                    return false;
			}
		}
		static data_t extract_forwarded_value(const Execution_Unit_Load_Store::Buffer_Entry& load, const Execution_Unit_Load_Store::Buffer_Entry& store) {
			u8 load_size{ access_size_from_mode(load.mode) };
			u32 byte_offset{ static_cast<u32>(load.calculated_address - store.calculated_address) };

			// Shift the store data so the byte(s) the load wants are at bit 0
			data_t raw{ (store.register_data.UNSIGNED >> (byte_offset * 8)) };

			// Mask to exactly load_size bytes
			data_t mask = { (load_size >= 4) ? 0xFFFFFFFFu : ((1u << (load_size * 8)) - 1u) };
			raw.UNSIGNED &= mask.UNSIGNED;

			// Sign-extend if needed (LB / LH)
			if (mode_is_signed(load.mode)) {
				u32 sign_bit = (load_size * 8) - 1;  // bit 7 or bit 15
				if (raw.UNSIGNED & (1u << sign_bit)) {
					raw.UNSIGNED |= ~mask.UNSIGNED; // fill upper bits with 1s
				}
			}

			return raw;
		}
		Execution_Result Execution_Unit_Load_Store::execute_load() {

			auto executable_load_index(find_load_that_is_executable());
			if (executable_load_index.first == Constants::EXECUTABLE_LOAD_DOES_NOT_EXIST)
				return { Constants::EXECUTION_RESULT_INVALID };

			if (executable_load_index.second == Constants::LOAD_DOES_NOT_USE_FORWARD_FROM_STORE) { // it is a bypassable load
				const Buffer_Entry* bypassable_load_entry{ &_load_buffer.at(executable_load_index.first) };
				data_t write_data{ DCache::read(bypassable_load_entry->mode,bypassable_load_entry->calculated_address) };
				//write to the physical register file
				Register_Manager::write(bypassable_load_entry->register_id, write_data);
				// make the rob entry ready
				Reorder_Buffer::set_ready(bypassable_load_entry->reorder_buffer_entry_index);
				Execution_Result result{
					Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD,
					write_data, // will be needed in forwarding to reservation stations
					bypassable_load_entry->producer_tag, // will be needed in forwarding logic 
				};
#ifdef DEBUG_PRINTS
				std::cout << std::format("Load instruction {} was executed using bypasssing.\n", bypassable_load_entry->timestamp);
#endif
				/* the load instruction in this buffer could be executed speculatively or earlier than a preceding store instruction
					so we are going to push it to the speculative load buffer
					the erasion of the speculated load buffer entry is going to happen when the reorder buffer retires the load instruction
				*/
				_speculative_load_buffer.emplace_back(_load_buffer[executable_load_index.first]);
				_load_buffer.erase(_load_buffer.begin() + executable_load_index.first);
				return result;
			}

			const Buffer_Entry* forwardable_load_entry{ &_load_buffer[executable_load_index.first] };
			const Buffer_Entry* store_buffer_entry_that_is_forwarded_from{ &_store_buffer[executable_load_index.second] };
			switch (classify_overlap(*forwardable_load_entry, *store_buffer_entry_that_is_forwarded_from)) {

			case STORE_FORWARD_RESULT::FULL_FORWARD:
			{
				data_t forwarded_value = extract_forwarded_value(*forwardable_load_entry, *store_buffer_entry_that_is_forwarded_from);

				Register_Manager::write(forwardable_load_entry->register_id, forwarded_value);
				Reorder_Buffer::set_ready(forwardable_load_entry->reorder_buffer_entry_index);

				Execution_Result result(
					Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD,
					forwarded_value,
					forwardable_load_entry->producer_tag
				);
#ifdef DEBUG_PRINTS
				std::cout << std::format("Load instruction {} was executed using forwarding from store instruction {}.\n", forwardable_load_entry->timestamp, store_buffer_entry_that_is_forwarded_from->timestamp);
#endif
				/* the load instruction in this buffer could be executed speculatively or earlier than a preceding store instruction
					so we are going to push it to the speculative load buffer
					the erasion of the speculated load buffer entry is going to happen when the reorder buffer retires the load instruction
				*/
				_speculative_load_buffer.emplace_back(_load_buffer[executable_load_index.first]);
				_load_buffer.erase(_load_buffer.begin() + executable_load_index.first);
				return result;
			}

			default:
				return { Constants::EXECUTION_RESULT_INVALID };
			}
		}

		void Execution_Unit_Load_Store::execute_store(u64 head) {
#ifdef DEBUG_PRINTS
			std::vector<size_t> commited_stores{};
#endif
			for (size_t i{}; i < _store_buffer.size(); i++) {
				const auto& store_entry{ _store_buffer[i] };
				if (store_entry.mode != EXECUTION_UNIT_MODE::UNKNOWN && store_entry.reorder_buffer_entry_index == head) {
					if (!(Screen_MMIO::handle_write(store_entry.calculated_address, store_entry.register_data.SIGNED)))
						DCache::write(store_entry.mode, store_entry.calculated_address, store_entry.register_data);
#ifdef DEBUG_PRINTS
					commited_stores.emplace_back(i);
#endif
				}
			}
#ifdef DEBUG_PRINTS
			for (auto j : commited_stores) {
				std::cout << std::format("Store instruction timestamp : {} wrote its data to the memory.\n", _store_buffer[j].timestamp);
			}
#endif
			std::erase_if(_store_buffer, [&](const auto& a) {return a.reorder_buffer_entry_index == head; });
		}

		time_t Execution_Unit_Load_Store::flush_mispredicted(time_t timestamp) {
			std::vector<time_t> erased_entry_timestamps{};
			std::erase_if(
				_load_buffer,
				[&](const Execution_Unit_Load_Store::Buffer_Entry& a) {
					if (a.timestamp > timestamp) {
						erased_entry_timestamps.emplace_back(a.timestamp);
						return true;
					}
					return false;
				}
			);

			std::erase_if(
				_speculative_load_buffer,
				[&](const Execution_Unit_Load_Store::Buffer_Entry& a) {
					if (a.timestamp > timestamp) {
						erased_entry_timestamps.emplace_back(a.timestamp);
						return true;
					}
					return false;
				}
			);
			std::erase_if(
				_store_buffer,
				[&](const Execution_Unit_Load_Store::Buffer_Entry& a) {
					if (a.timestamp > timestamp) {
						erased_entry_timestamps.emplace_back(a.timestamp);
						return true;
					}
					return false;
				}
			);
			auto it(std::max_element(erased_entry_timestamps.begin(), erased_entry_timestamps.end()));
			if (it != erased_entry_timestamps.end())
				return *it;
			return Constants::TIME_ZERO;
		}

		void Execution_Unit_Load_Store::reset() {
			_load_buffer.clear();
			_store_buffer.clear();
		}



		Execution_Unit_Load_Store::Buffer_Entry* Execution_Unit_Load_Store::allocate_store_buffer_entry() {
			return &_store_buffer.emplace_back();
		}

		Execution_Unit_Load_Store::Buffer_Entry* Execution_Unit_Load_Store::allocate_load_buffer_entry() {
			return &_load_buffer.emplace_back();
		}

		void Execution_Unit_Load_Store::remove_speculated_load(u64 reorder_buffer_entry_index) {
			std::erase_if(
				_speculative_load_buffer,
				[&](const Buffer_Entry& entry) {return entry.reorder_buffer_entry_index == reorder_buffer_entry_index; }
			);
		}
		Execution_Result Execution_Unit_Load_Store::resolve_speculated_loads(u64 head_that_points_to_rob) {
			bool misspeculated{ false };
			const auto& store_buffer_entry{
				std::ranges::find_if(
					_store_buffer,
					[&](const auto& entry) {
						return entry.reorder_buffer_entry_index == head_that_points_to_rob && entry.mode != EXECUTION_UNIT_MODE::UNKNOWN;
					}
				)
			};
			std::vector<const Buffer_Entry*> misspeculated_loads;
			for (const auto& speculated_load : _speculative_load_buffer) {
				// resolving of the speculated instruction is done by the store instructions that precede the speculated store instruction
				if (store_buffer_entry->store_id > speculated_load.store_id)
					continue;
				// misspeculated means the load executed earlier than the preceding store
				if (store_buffer_entry->calculated_address == speculated_load.calculated_address) {
#ifdef DEBUG_PRINTS
					std::cout << Constants::RED << std::format("Load instruction instruction timestamp:{} was misspeculated\n", speculated_load.timestamp) << Constants::RESET;
#endif
					Reservation_Station_Pool::flush_mispredicted(0xFFFFFFFF, speculated_load.timestamp);
					Execution_Unit_Load_Store::flush_mispredicted(speculated_load.timestamp);
					Reorder_Buffer::set_speculation_evaluation(
						speculated_load.reorder_buffer_entry_index,
						true,
						Constants::END_OF_TIME
					);
					misspeculated_loads.push_back(&speculated_load);
					misspeculated = true;
				}
			}
			if (misspeculated) {
				/* out of misspeculated loads we gotta set the program counter to the earliest instruction by timestamp
					one might ask what if the misspeculated load instructions have the same timestamp?
					The answer is they dont! when a load instruction is fetched the timestamp of the next instruction is immediately updated
				*/
				auto earliest_load_it{
					std::min_element(
						misspeculated_loads.begin(),
						misspeculated_loads.end(),
						[](const auto& a, const auto& b) {
							return a->timestamp < b->timestamp;
						}
					)
				};
				Fetch_Unit::set_program_counter((*earliest_load_it)->instruction_address);
				Fetch_Group::group = std::vector<Fetch_Element>(Constants::FETCH_WIDTH);
				Fetch_Unit::stall();
			}
			return { Constants::EXECUTION_RESULT_INVALID, {},Constants::NO_PRODUCER_TAG, misspeculated };
		}
		Execution_Result Execution_Unit_Branch::execute(const Reservation_Station_Entry* source_entry) {

			if (!source_entry)
				return { Constants::EXECUTION_RESULT_INVALID };
			memory_addr_t target_address{};
			bool actual_taken{ false };
			switch (source_entry->mode) {
			case EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JALR:// Fetch unit keeps fetching when it sees jalr instruction so we gotta recover
			{
				target_address = source_entry->src1.UNSIGNED + source_entry->src2.UNSIGNED;
				Fetch_Unit::set_program_counter(target_address);
				Fetch_Unit::set_program_counter_flags();
				Register_Manager::write(source_entry->destination_register_id, { source_entry->instruction_address + 1 });
				Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
				time_t latest_flushed_reservation_station_entry_timestamp{ Reservation_Station_Pool::flush_mispredicted(source_entry->self_tag,source_entry->timestamp) };
				time_t latest_flushed_load_store_buffer_entry_timestamp{ Execution_Unit_Load_Store::flush_mispredicted(source_entry->timestamp) };
				Reorder_Buffer::set_speculation_evaluation(
					source_entry->reorder_buffer_entry_index,
					true,
					std::max(latest_flushed_reservation_station_entry_timestamp, latest_flushed_load_store_buffer_entry_timestamp)
				);
				Fetch_Group::group = std::vector<Fetch_Element>(Constants::FETCH_WIDTH);
				return {
					Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD,
					source_entry->instruction_address + 1, // will be needed in forwarding to reservation stations
					source_entry->self_tag,// will be needed in forwarding logic 
					true // detected misprediction
				};
			}
			break;
			case EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JAL: // Fetch unit already jumped so all we have to do is write the values to the registers
				Register_Manager::write(source_entry->destination_register_id, { source_entry->instruction_address + 1 });
				Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
				return {
					Constants::EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD,
					source_entry->instruction_address + 1, // will be needed in forwarding to reservation stations
					source_entry->self_tag, // will be needed in forwarding logic 
				};
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_EQUAL:
				actual_taken = source_entry->src1.SIGNED == source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_NOT_EQUAL:
				actual_taken = source_entry->src1.SIGNED != source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN:
				actual_taken = source_entry->src1.SIGNED < source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_THAN:
				actual_taken = source_entry->src1.SIGNED > source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN:
				actual_taken = source_entry->src1.SIGNED >= source_entry->src2.SIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN_UNSIGNED:
				actual_taken = source_entry->src1.UNSIGNED >= source_entry->src2.UNSIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN_UNSIGNED:
				actual_taken = source_entry->src1.UNSIGNED < source_entry->src2.UNSIGNED;
				break;
			case EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_THAN_UNSIGNED:
				actual_taken = source_entry->src1.UNSIGNED > source_entry->src2.UNSIGNED;
				break;
				// wont happen
			default:
				break;

			}
			bool prediction(source_entry->fetch_unit_prediction & Constants::PREDICTED_TAKEN);
			// update pht
			Fetch_Unit::update_pattern_history_table(source_entry->instruction_address, actual_taken);
			target_address = source_entry->branch_target;
			Fetch_Unit::create_btb_entry(source_entry->instruction_address, target_address);
			Reorder_Buffer::set_ready(source_entry->reorder_buffer_entry_index);
			if (prediction == actual_taken) {
				Reorder_Buffer::set_speculation_evaluation(source_entry->reorder_buffer_entry_index, false, Constants::TIME_ZERO);
#ifdef DEBUG_PRINTS
				std::cout << Constants::GREEN << "Instructions[" << source_entry->instruction_address << "] timestamp : " << source_entry->timestamp << " was predicted correctly\n" << Constants::RESET;
#endif
				return { Constants::EXECUTION_RESULT_STATION_DEALLOCATE_ONLY,0,source_entry->self_tag, false };
			}
			// misprediction recovery

#ifdef DEBUG_PRINTS
			std::cout << Constants::RED << "Instructions[" << source_entry->instruction_address << "] timestamp : " << source_entry->timestamp << " was mispredicted\n"
				<< std::format("Prediction : {}, Actual : {} ", prediction, actual_taken) <<
				"Setting PC to : " << target_address << "\n" <<
				Constants::RESET;
#endif
			Fetch_Group::group = std::vector<Fetch_Element>(Constants::FETCH_WIDTH);
			if (actual_taken == true && prediction == false) {
				Fetch_Unit::set_program_counter(target_address);
			}
			else
				Fetch_Unit::set_program_counter(source_entry->instruction_address + 1);
			Fetch_Unit::set_program_counter_flags();
			time_t latest_flushed_reservation_station_entry_timestamp{ Reservation_Station_Pool::flush_mispredicted(source_entry->self_tag,source_entry->timestamp) };
			time_t latest_flushed_load_store_buffer_entry_timestamp{ Execution_Unit_Load_Store::flush_mispredicted(source_entry->timestamp) };
			Reorder_Buffer::set_speculation_evaluation(
				source_entry->reorder_buffer_entry_index,
				true,
				std::max(latest_flushed_reservation_station_entry_timestamp, latest_flushed_load_store_buffer_entry_timestamp)
			);
			return { Constants::EXECUTION_RESULT_STATION_DEALLOCATE_ONLY,0,source_entry->self_tag, true };
		}

	} // namespace Core
} // namespace OoOVis