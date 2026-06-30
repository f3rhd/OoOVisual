// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Commit/ReorderBuffer.h>
#include <Core/Commit/ReorderBufferEntry.h>
#include <Core/Constants/Constants.h>
#include <Core/Constants/Constants.h>
#include <Core/Dispatch/Dispatcher.h>
#include <Core/Execution/ExecutionUnitModes.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/Fetch/FetchElements.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/ReservationStation/ReservationStation.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Types/Types.h>
#include <Frontend/Parser/Instruction.h>
#include <Visualizer/App.h>
#include <iostream>
#include <map>
#include <memory>
#include <array>
#ifdef DEBUG_PRINTS
#include <format>
#endif
namespace OoOVisual
{
	namespace Core
	{
		DISPATCH_FEEDBACK Dispatcher::dispatch_fetch_element(const Fetch_Element& fetch_element) {
			DISPATCH_FEEDBACK feedback{};
			RESERVATION_STATION_ID station_id{};
			if (!fetch_element.instruction) {
				feedback = DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
				return feedback;
			}
			if (Reorder_Buffer::full()) {
				feedback = DISPATCH_FEEDBACK::REORDER_BUFFER_WAS_FULL;
			}
			else {
				switch ((*fetch_element.instruction)->flow()) {
				case FrontEnd::FLOW_TYPE::REGISTER:
					station_id = get_register_instruction_reservation_station_id((*fetch_element.instruction).get());
					feedback = dispatch_register_instruction(fetch_element, Reservation_Station_Pool::get_reservation_station(station_id));
					break;
				case FrontEnd::FLOW_TYPE::LOAD:
					station_id = RESERVATION_STATION_ID::LOAD_STORE;
					feedback = dispatch_load_instruction(fetch_element, Reservation_Station_Pool::get_reservation_station(RESERVATION_STATION_ID::LOAD_STORE));
					break;
				case FrontEnd::FLOW_TYPE::STORE:
					station_id = RESERVATION_STATION_ID::LOAD_STORE;
					feedback = dispatch_store_instruction(fetch_element, Reservation_Station_Pool::get_reservation_station(RESERVATION_STATION_ID::LOAD_STORE));
					break;
				case FrontEnd::FLOW_TYPE::BRANCH_CONDITIONAL:
					station_id = RESERVATION_STATION_ID::BRANCH;
					feedback = dispatch_branch_instruction(fetch_element, Reservation_Station_Pool::get_reservation_station(RESERVATION_STATION_ID::BRANCH));
					break;
				case FrontEnd::FLOW_TYPE::BRANCH_UNCONDITIONAL:
					station_id = RESERVATION_STATION_ID::BRANCH;
					feedback = dispatch_jump_instruction(fetch_element, Reservation_Station_Pool::get_reservation_station(RESERVATION_STATION_ID::BRANCH));
					break;
				case FrontEnd::FLOW_TYPE::UNKNOWN:
				default:
					std::cout << "Faced with unknown instruction flow type\n";
					Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
				}
			}

			_station_dispatch_map.at(station_id) = feedback;
			return feedback;
		}
		RESERVATION_STATION_ID Dispatcher::get_register_instruction_reservation_station_id(FrontEnd::Instruction* instruction) {
			if (const auto* register_instruction = dynamic_cast<FrontEnd::Register_Instruction*>(instruction)) {
				switch (register_instruction->instruction_type()) {
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SUB:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::LOAD_UPPER:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::AUIPC:
					return RESERVATION_STATION_ID::ADD_SUB;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIV:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIVU:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::REM:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::REMU:
					return RESERVATION_STATION_ID::DIVIDER;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MUL:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULH:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULHSU:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULHU:
					return RESERVATION_STATION_ID::MULTIPLIER;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::AND:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::OR:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::XOR:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLL:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRL:
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRA:
					return RESERVATION_STATION_ID::BITWISE;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT:
					return RESERVATION_STATION_ID::SET_LESS;
					break;
				default: // shouldn't happen
					break;
				}
			}
			return RESERVATION_STATION_ID::UNKNOWN;
		}
		DISPATCH_FEEDBACK Dispatcher::dispatch_register_instruction(const Fetch_Element& fetch_element, Reservation_Station& station) {
			const auto& instruction = *fetch_element.instruction;
			const auto& instruction_id = fetch_element.address;
			if (const auto* register_instruction{ dynamic_cast<FrontEnd::Register_Instruction*>(instruction.get()) }) {
				if (register_instruction->dest_reg() == 0)
					return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
				if (station.full())
					return DISPATCH_FEEDBACK::RESERVATION_STATION_WAS_FULL;
				if (Register_Manager::full()) return DISPATCH_FEEDBACK::REGISTER_FILE_WAS_FULL;

				Reservation_Station_Entry* allocated_reservation_station_entry{ station.allocate_entry() };
				switch (register_instruction->instruction_type()) {
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::ADD_SUB_UNIT_ADD;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SUB:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::ADD_SUB_UNIT_SUB;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::LOAD_UPPER:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::ADD_SUB_UNIT_LOAD_UPPER;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::AUIPC:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::ADD_SUB_UNIT_AUIPC;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIV:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_SIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIVU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_UNSIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::REM:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_SIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::REMU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_UNSIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MUL:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_SIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULH:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULHSU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_SIGNED_UNSIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULHU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_UNSIGNED;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::AND:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BITWISE_AND;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::OR:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BITWISE_XOR;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::XOR:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOGICAL_XOR;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLL:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BITWISE_SHIFT_LEFT_LOGICAL;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRL:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_LOGICAL;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRA:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_ARITHMETIC;
					break;
				case FrontEnd::Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::SET_LESS_THAN;
					break;
				default: // shouldn't happen
					break;
				}
				Physical_Register_File_Entry entry(Register_Manager::read_with_alias(register_instruction->src1_reg()));
				allocated_reservation_station_entry->producer_tag1 = entry.producer_tag;
				allocated_reservation_station_entry->src1 = entry.data;
				// store the old alias before renaming
				auto old_alias{ Register_Manager::aliasof(register_instruction->dest_reg()) };
				if (register_instruction->uses_immval()) {
					allocated_reservation_station_entry->src2.SIGNED = register_instruction->src2().imm_val;
					allocated_reservation_station_entry->producer_tag2 = Constants::NO_PRODUCER_TAG;
				}
				else {
					allocated_reservation_station_entry->src2 = Register_Manager::read_with_alias(register_instruction->src2().src2_reg).data;
					allocated_reservation_station_entry->producer_tag2 = Register_Manager::read_with_alias(register_instruction->src2().src2_reg).producer_tag;
				}
				reg_id_t destination_physical_register_id{ Register_Manager::allocate_physical_register_for(register_instruction->dest_reg(), allocated_reservation_station_entry->self_tag) }; // we do the fulness checking before calling this, so we good
				// allocate reorder buffer before renaming the destination register, this function call is invoked in the case where Reorder_Buffer is not full
				auto target_reorder_buffer_entry_index{ Reorder_Buffer::allocate(
						std::make_unique<Register_Reorder_Buffer_Entry>(
							register_instruction->flow(),
							register_instruction->dest_reg(),
							old_alias,
							destination_physical_register_id,
							fetch_element.timestamp
						)
					)
				};
				allocated_reservation_station_entry->ready = allocated_reservation_station_entry->producer_tag1 == Constants::NO_PRODUCER_TAG && allocated_reservation_station_entry->producer_tag2 == Constants::NO_PRODUCER_TAG;
				allocated_reservation_station_entry->busy = true;
				allocated_reservation_station_entry->reorder_buffer_entry_index = target_reorder_buffer_entry_index;
				allocated_reservation_station_entry->destination_register_id = destination_physical_register_id;
				allocated_reservation_station_entry->instruction_address = instruction_id;
				allocated_reservation_station_entry->timestamp = fetch_element.timestamp;
#ifdef DEBUG_PRINTS
				std::cout << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}].\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag);
#endif
			}
			else {
				std::cout << "Dynamic cast of Instruction -> Register_Instruction failed.\n";
				Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
			return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
		}
		DISPATCH_FEEDBACK Dispatcher::dispatch_load_instruction(const Fetch_Element& fetch_element, Reservation_Station& station) {
			const auto& instruction = *fetch_element.instruction;
			const auto& instruction_id = fetch_element.address;
			if (const auto* load_instruction{ dynamic_cast<FrontEnd::Load_Instruction*>(instruction.get()) }) {
				if (load_instruction->dest_reg() == 0)
					return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
				if (station.full())
					return DISPATCH_FEEDBACK::RESERVATION_STATION_WAS_FULL;
				if (Execution_Unit_Load_Store::load_buffer_is_full())
					return DISPATCH_FEEDBACK::LOAD_BUFFER_WAS_FULL;
				if (Register_Manager::full())
					return DISPATCH_FEEDBACK::REGISTER_FILE_WAS_FULL;
				Reservation_Station_Entry* allocated_reservation_station_entry{ station.allocate_entry() };
				switch (load_instruction->kind()) {
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::LB:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE;
					break;
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::LH:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF;
					break;
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::LW:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_WORD;
					break;
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::LBU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE_UNSIGNED;
					break;
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::LHU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF_UNSIGNED;
					break;
				case FrontEnd::Load_Instruction::LOAD_INSTRUCTION_TYPE::UNKNOWN:
				default:
					// shouldnt happen
					std::cout << "Faced with unkown load instruction type while dispatching.\n";
					Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
					break;

				}
				Physical_Register_File_Entry entry(Register_Manager::read_with_alias(load_instruction->base_reg()));

				auto old_alias{ Register_Manager::aliasof(load_instruction->dest_reg()) };
				// allocate reorder buffer before renaming the destination register, this function call is invoked in the case where Reorder_Buffer is not full
				reg_id_t destination_physical_register_id = Register_Manager::allocate_physical_register_for(load_instruction->dest_reg(), allocated_reservation_station_entry->self_tag);
				auto target_reorder_buffer_entry_index{ Reorder_Buffer::allocate(
					std::make_unique<Load_Reorder_Buffer_Entry>(
						load_instruction->flow(),
						load_instruction->dest_reg(),
						old_alias,
						destination_physical_register_id,
						fetch_element.timestamp
					)
				)
				};
				allocated_reservation_station_entry->src1 = entry.data;
				allocated_reservation_station_entry->producer_tag1 = entry.producer_tag;
				allocated_reservation_station_entry->destination_register_id = load_instruction->dest_reg();
				allocated_reservation_station_entry->src2.SIGNED = (load_instruction->offset());
				allocated_reservation_station_entry->producer_tag2 = Constants::NO_PRODUCER_TAG;
				allocated_reservation_station_entry->ready = allocated_reservation_station_entry->producer_tag1 == Constants::NO_PRODUCER_TAG && allocated_reservation_station_entry->producer_tag2 == Constants::NO_PRODUCER_TAG;
				allocated_reservation_station_entry->self_tag = allocated_reservation_station_entry->self_tag;
				allocated_reservation_station_entry->busy = true;
				allocated_reservation_station_entry->reorder_buffer_entry_index = target_reorder_buffer_entry_index;
				allocated_reservation_station_entry->destination_register_id = destination_physical_register_id;
				allocated_reservation_station_entry->instruction_address = instruction_id;
				allocated_reservation_station_entry->timestamp = fetch_element.timestamp;
				allocated_reservation_station_entry->store_id = _last_store_id;
				Execution_Unit_Load_Store::Buffer_Entry* allocated_load_buffer_entry{ Execution_Unit_Load_Store::allocate_load_buffer_entry() };
				allocated_load_buffer_entry->reorder_buffer_entry_index = allocated_reservation_station_entry->reorder_buffer_entry_index;
				allocated_load_buffer_entry->timestamp = allocated_reservation_station_entry->timestamp;
				allocated_load_buffer_entry->instruction_address = allocated_reservation_station_entry->instruction_address;
#ifdef DEBUG_PRINTS
				std::cout << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}].\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag);
#endif

			}
			// shouldnt happennnn but again..
			else {
				std::cout << "Dynamic cast of Instruction -> Load_Instruction failed\n";
				Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;

			}
			return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
		}
		u32 Dispatcher::_last_store_id = 0;
		DISPATCH_FEEDBACK Dispatcher::dispatch_store_instruction(const Fetch_Element& fetch_element, Reservation_Station& station) {
			const auto& instruction = *fetch_element.instruction;
			const auto& instruction_id = fetch_element.address;
			if (const auto* store_instruction = dynamic_cast<FrontEnd::Store_Instruction*>(instruction.get())) {
				if (station.full())
					return DISPATCH_FEEDBACK::RESERVATION_STATION_WAS_FULL;
				if (Execution_Unit_Load_Store::store_buffer_is_full())
					return DISPATCH_FEEDBACK::STORE_BUFFER_WAS_FULL; // 
				Reservation_Station_Entry* allocated_reservation_station_entry(station.allocate_entry());
				switch (store_instruction->kind()) {
				case FrontEnd::Store_Instruction::STORE_INSTRUCTION_TYPE::SB:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_STORE_BYTE;
					break;
				case FrontEnd::Store_Instruction::STORE_INSTRUCTION_TYPE::SH:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_STORE_HALF;
					break;
				case FrontEnd::Store_Instruction::STORE_INSTRUCTION_TYPE::SW:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::LOAD_STORE_STORE_WORD;
					break;
					// shouldnt happen
				case FrontEnd::Store_Instruction::STORE_INSTRUCTION_TYPE::UNKNOWN:
				default:
					std::cout << "Faced with an unknown store instruction type while dispatching.\n";
					Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
					break;

				}
				Physical_Register_File_Entry src1_entry(Register_Manager::read_with_alias(store_instruction->src1()));
				Physical_Register_File_Entry src2_entry(Register_Manager::read_with_alias(store_instruction->src2()));
				allocated_reservation_station_entry->producer_tag1 = src1_entry.producer_tag;
				allocated_reservation_station_entry->producer_tag2 = src2_entry.producer_tag;
				allocated_reservation_station_entry->src1 = src1_entry.data;
				allocated_reservation_station_entry->src2 = src2_entry.data;
				allocated_reservation_station_entry->destination_register_id = static_cast<reg_id_t>(store_instruction->offset()); // the offset lives in destination id here
				allocated_reservation_station_entry->destination_register_id_as_ofsset = true;
				allocated_reservation_station_entry->self_tag = allocated_reservation_station_entry->self_tag;
				allocated_reservation_station_entry->busy = true;
				allocated_reservation_station_entry->ready = allocated_reservation_station_entry->producer_tag1 == Constants::NO_PRODUCER_TAG && allocated_reservation_station_entry->producer_tag2 == Constants::NO_PRODUCER_TAG;
				allocated_reservation_station_entry->timestamp = fetch_element.timestamp;
				allocated_reservation_station_entry->store_id = _last_store_id++;
				allocated_reservation_station_entry->instruction_address = instruction_id; // this id will be passed down to the entry in the store buffer
				// when the store instruction is in the store buffer it will wait a signal from reorder buffer to finish its execution architecturally, that is why we need store_id
				allocated_reservation_station_entry->reorder_buffer_entry_index = static_cast<u32>(Reorder_Buffer::allocate(
					std::make_unique<Store_Reorder_Buffer_Entry>(
						store_instruction->flow(),
						allocated_reservation_station_entry->store_id,
						fetch_element.timestamp
					)
				)
					);
				Execution_Unit_Load_Store::Buffer_Entry* store_entry{ Execution_Unit_Load_Store::allocate_store_buffer_entry() };
				store_entry->reorder_buffer_entry_index = allocated_reservation_station_entry->reorder_buffer_entry_index;
				store_entry->timestamp = allocated_reservation_station_entry->timestamp;
#ifdef DEBUG_PRINTS
				std::cout << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}].\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag);
#endif
			}
			else {
				std::cout << "Dynamic cast of Instruction -> Store_Instruction failed\n";
				Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
			return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
		}
		DISPATCH_FEEDBACK Dispatcher::dispatch_branch_instruction(const Fetch_Element& fetch_element, Reservation_Station& station) {
			const auto& instruction = *fetch_element.instruction;
			const auto& instruction_id = fetch_element.address;
			if (const auto* branch_instruction{ dynamic_cast<FrontEnd::Branch_Instruction*>(instruction.get()) }) {
				if (station.full())
					return DISPATCH_FEEDBACK::RESERVATION_STATION_WAS_FULL;
				Reservation_Station_Entry* allocated_reservation_station_entry(station.allocate_entry());
				allocated_reservation_station_entry->busy = true;
				allocated_reservation_station_entry->self_tag = allocated_reservation_station_entry->self_tag;
				allocated_reservation_station_entry->reorder_buffer_entry_index = Reorder_Buffer::allocate(
					std::make_unique<Branch_Conditional_Reorder_Buffer_Entry>(
						branch_instruction->flow(),
						fetch_element.timestamp
					)
				);
				Physical_Register_File_Entry entry1(Register_Manager::read_with_alias(branch_instruction->src1()));
				Physical_Register_File_Entry entry2(Register_Manager::read_with_alias(branch_instruction->src2()));
				switch (branch_instruction->kind()) {
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BEQ:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_EQUAL;
					break;
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BNE:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_NOT_EQUAL;
					break;
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGE:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN;
					break;
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGEU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN_UNSIGNED;
					break;
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLT:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN;
					break;
				case FrontEnd::Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLTU:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN_UNSIGNED;
					break;
				default:
					break;
				}
				allocated_reservation_station_entry->producer_tag1 = entry1.producer_tag;
				allocated_reservation_station_entry->producer_tag2 = entry2.producer_tag;
				allocated_reservation_station_entry->src1 = entry1.data;
				allocated_reservation_station_entry->src2 = entry2.data;
				allocated_reservation_station_entry->ready = allocated_reservation_station_entry->producer_tag1 == Constants::NO_PRODUCER_TAG && allocated_reservation_station_entry->producer_tag2 == Constants::NO_PRODUCER_TAG;
				// execution stage will need these for branch instruction
				allocated_reservation_station_entry->branch_target = branch_instruction->target_addr();
				allocated_reservation_station_entry->instruction_address = instruction_id;
				allocated_reservation_station_entry->timestamp = fetch_element.timestamp;
				allocated_reservation_station_entry->fetch_unit_prediction = fetch_element.branch_prediction;
#ifdef DEBUG_PRINTS
				if (allocated_reservation_station_entry->fetch_unit_prediction == Constants::PREDICTED_NOT_TAKEN)
					std::cout << Constants::YELLOW << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}] Fetch_Unit_Prediction : NOT_TAKEN  .\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag) << Constants::RESET;
				else if (allocated_reservation_station_entry->fetch_unit_prediction == Constants::PREDICTED_TAKEN)
					std::cout << Constants::YELLOW << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}] Fetch_Unit_Prediction : TAKEN  .\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag) << Constants::RESET;
#endif
			}
			else {
				std::cout << "Dynamic cast of Instruction -> Branch_Instruction failed\n";
				Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
			return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;
		}
		DISPATCH_FEEDBACK Dispatcher::dispatch_jump_instruction(const Fetch_Element& fetch_element, Reservation_Station& station) {

			const auto& instruction = *fetch_element.instruction;
			const auto& instruction_id = fetch_element.address;
			if (const auto* jump_instruction = dynamic_cast<FrontEnd::Jump_Instruction*>(instruction.get())) {

				if (station.full())
					return DISPATCH_FEEDBACK::REORDER_BUFFER_WAS_FULL;
				Reservation_Station_Entry* allocated_reservation_station_entry(station.allocate_entry());
				switch (jump_instruction->kind()) {
				case FrontEnd::Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JALR;
					break;
				case FrontEnd::Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL:
					allocated_reservation_station_entry->mode = EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JAL;
					break;
				default: // wont happen
					break;
				}
				allocated_reservation_station_entry->self_tag = allocated_reservation_station_entry->self_tag;
				allocated_reservation_station_entry->busy = true;
				allocated_reservation_station_entry->timestamp = fetch_element.timestamp;
				reg_id_t old_alias = Register_Manager::aliasof(jump_instruction->dest_reg());
				Physical_Register_File_Entry entry{ Register_Manager::read_with_alias(jump_instruction->src1()) };
				reg_id_t destination_pysical_register_id = Register_Manager::allocate_physical_register_for(jump_instruction->dest_reg(), allocated_reservation_station_entry->self_tag);
				allocated_reservation_station_entry->destination_register_id = destination_pysical_register_id;
				u64 target_reorder_buffer_entry_index{};
				switch (jump_instruction->kind()) {
				case FrontEnd::Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR:
					allocated_reservation_station_entry->producer_tag1 = entry.producer_tag;
					allocated_reservation_station_entry->src1 = entry.data;
					allocated_reservation_station_entry->producer_tag2 = Constants::NO_PRODUCER_TAG;
					allocated_reservation_station_entry->src2.SIGNED = jump_instruction->offset();
					allocated_reservation_station_entry->ready = allocated_reservation_station_entry->producer_tag1 == Constants::NO_PRODUCER_TAG;
					target_reorder_buffer_entry_index = Reorder_Buffer::allocate(
						std::make_unique<Branch_Unconditional_Reorder_Buffer_Entry>(
							jump_instruction->flow(),
							jump_instruction->dest_reg(),
							old_alias,
							destination_pysical_register_id,
							true, // always mispredicted
							fetch_element.timestamp
						)
					);
					allocated_reservation_station_entry->reorder_buffer_entry_index = target_reorder_buffer_entry_index;
					break;
				case FrontEnd::Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL:
					allocated_reservation_station_entry->ready = true;
					target_reorder_buffer_entry_index = Reorder_Buffer::allocate(
						std::make_unique<Branch_Unconditional_Reorder_Buffer_Entry>(
							jump_instruction->flow(),
							jump_instruction->dest_reg(),
							old_alias,
							destination_pysical_register_id,
							false, // never mispredicted
							fetch_element.timestamp
						)
					);
					allocated_reservation_station_entry->reorder_buffer_entry_index = target_reorder_buffer_entry_index;
					break;
				default:
					std::cout << "Faced with unknown jump instruction type while dispatching.\n";
					Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
					break;
				}
				allocated_reservation_station_entry->instruction_address = instruction_id;
#ifdef DEBUG_PRINTS
				std::cout << std::format("Dispatched Instructions[{}] timestamp : {} to ReservationStationPool[{}].\n", instruction_id, allocated_reservation_station_entry->timestamp, allocated_reservation_station_entry->self_tag);
#endif
			}
			else {
				std::cout << "Dynamic cast of Instruction -> Jump_Instruction failed\n";
				Visualizer::App::close(); return DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
			return DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH;

		}

		std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH> Dispatcher::_last_dispatch_feedback{};

		const std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH>& Dispatcher::last_dispatch_feedback() {
			return _last_dispatch_feedback;
		}

		DISPATCH_FEEDBACK Dispatcher::dispatch_feedback_of(RESERVATION_STATION_ID id) {
			return _station_dispatch_map.at(id);
		}

		void Dispatcher::reset() {
			_last_dispatch_feedback = std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH>{};
			_last_store_id = 0;
			for (auto& [key, val] : _station_dispatch_map) {
				val = DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
		}

		std::map<RESERVATION_STATION_ID, DISPATCH_FEEDBACK> Dispatcher::_station_dispatch_map{
			{RESERVATION_STATION_ID::ADD_SUB,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::BITWISE,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::SET_LESS,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::MULTIPLIER,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::DIVIDER,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::LOAD_STORE,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
			{RESERVATION_STATION_ID::BRANCH,DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH},
		};
		std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH> Dispatcher::dispatch_fetch_group() {
			std::array<DISPATCH_FEEDBACK, Constants::FETCH_WIDTH> feedback{};
			for (auto& [key, val] : _station_dispatch_map) {
				val = DISPATCH_FEEDBACK::NO_INSTRUCTION_TO_DISPATCH;
			}
			if (Fetch_Group::group.empty()) {
				_last_dispatch_feedback = feedback;
				return feedback;
			}
			for (size_t i{}; i < Constants::FETCH_WIDTH; i++) {
				feedback[i] = dispatch_fetch_element(Fetch_Group::group[i]);
				if (feedback[i] != DISPATCH_FEEDBACK::SUCCESSFUL_DISPATCH) {
					break;
				}
			}
			_last_dispatch_feedback = feedback;
			return feedback;
		}

	} // namespace Core

} // namespace OoOVis
