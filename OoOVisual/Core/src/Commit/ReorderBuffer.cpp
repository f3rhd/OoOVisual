// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Commit/ReorderBuffer.h>
#include <Core/Commit/ReorderBufferEntry.h>
#include <Core/Constants/Constants.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/Types/Types.h>
#include <Frontend/Parser/Instruction.h>
#include <Visualizer/App.h>
#include <iostream>
#include <memory>
#include <utility>
#include <array>
#ifdef DEBUG_PRINTS
#include <format>
#endif
namespace OoOVisual
{
	namespace Core
	{
#define ROB_MISPREDICTION_RECOVERY(ENTRY_TYPE) {                                \
			auto branch_timestamp = dynamic_cast<ENTRY_TYPE*>(entry)->self_timestamp; \
			auto flush_boundary{ dynamic_cast<ENTRY_TYPE*>(entry)->flush_timestamp_boundary }; \
			if (flush_boundary == Constants::TIME_ZERO) {                                \
				_buffer[_head].reset();                                                  \
				_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;                    \
			}                                                                            \
			else {                                                                       \
				while (_buffer[_head] &&                                                 \
					_buffer[_head]->self_timestamp >= branch_timestamp &&                \
					_buffer[_head]->self_timestamp <= flush_boundary                     \
				) {                                                                      \
					_buffer[_head].reset();                                              \
					_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;                \
				}                                                                        \
			}                                                                            \
			Register_Manager::restore_alias_table();                                        \
		}
		std::array<std::unique_ptr<Reorder_Buffer_Entry>, Constants::REORDER_BUFFER_SIZE> Reorder_Buffer::_buffer{};
		size_t Reorder_Buffer::_head{};
		size_t Reorder_Buffer::_tail{};
		bool Reorder_Buffer::_head_moved{ false };
		bool Reorder_Buffer::_flushed{ false };

		void Reorder_Buffer::commit() {
			_head_moved = false;
			_flushed = false;
			for (size_t i{}; i < Constants::COMMIT_WIDTH; i++) {
				auto* entry = _buffer[_head].get();
				if (entry == nullptr) return;
				if (!entry->ready) return;
				_head_moved = true;
				switch (entry->flow()) {
				case FrontEnd::FLOW_TYPE::REGISTER:
				{
					auto entry_{ dynamic_cast<Register_Reorder_Buffer_Entry*>(entry) };
					Register_Manager::deallocate((entry_)->old_alias);
					Register_Manager::update_retirement_alias_table_with(
						(entry_)->architectural_register_id,
						(entry_)->new_alias
					);
					_buffer[_head].reset();
					_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;
					break;
				}
				case FrontEnd::FLOW_TYPE::LOAD:
				{
					auto entry_{ dynamic_cast<Load_Reorder_Buffer_Entry*>(entry) };
					Execution_Unit_Load_Store::remove_speculated_load(_head);
					if (entry_->misspeculated) {
						ROB_MISPREDICTION_RECOVERY(Load_Reorder_Buffer_Entry);
					}
					else {
						Register_Manager::deallocate((entry_)->old_alias);
						Register_Manager::update_retirement_alias_table_with(
							(entry_)->architectural_register_id,
							(entry_)->new_alias
						);
#ifdef DEBUG_PRINTS
						std::cout << Constants::GREEN << std::format("Load instruction timestamp: {} was speculated correctly\n", entry_->self_timestamp) << Constants::RESET;
#endif
						_buffer[_head].reset();
						_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;
					}
					break;
				}
				case FrontEnd::FLOW_TYPE::STORE:
				{
					Execution_Unit_Load_Store::resolve_speculated_loads(_head);
					Execution_Unit_Load_Store::execute_store(_head);
					_buffer[_head].reset();
					_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;
					break;
				}
				case FrontEnd::FLOW_TYPE::BRANCH_CONDITIONAL:

					if (dynamic_cast<Branch_Conditional_Reorder_Buffer_Entry*>(entry)->mispredicted) {
						ROB_MISPREDICTION_RECOVERY(Branch_Conditional_Reorder_Buffer_Entry);
						_flushed = true;
					}
					else {
						_buffer[_head].reset();
						_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;
					}
					break;
				case FrontEnd::FLOW_TYPE::BRANCH_UNCONDITIONAL:
				{
					auto* entry_{ dynamic_cast<Branch_Unconditional_Reorder_Buffer_Entry*>(entry) };
					Register_Manager::deallocate((entry_)->old_alias);
					Register_Manager::update_retirement_alias_table_with(
						(entry_)->architectural_register_id,
						(entry_)->new_alias
					);
					if ((entry_)->mispredicted) {
						ROB_MISPREDICTION_RECOVERY(Branch_Unconditional_Reorder_Buffer_Entry);
						_flushed = true;
					}
					else {
						_buffer[_head].reset();
						_head = (_head + 1) % Constants::REORDER_BUFFER_SIZE;
					}
					break;

				}
				default:
					break;

				}
			}
		}



		size_t Reorder_Buffer::allocate(std::unique_ptr<Reorder_Buffer_Entry>&& entry) {
			_buffer[_tail] = std::move(entry);
			auto allocated_entry_index = _tail;
			_tail = (_tail + 1) % Constants::REORDER_BUFFER_SIZE;
			return allocated_entry_index;
		}

		void Reorder_Buffer::set_ready(u64 target_entry_index) {
			if (target_entry_index >= Constants::REORDER_BUFFER_SIZE) {
				std::cout << "Tried to access non-existing reorder buffer entry.\n";
				Visualizer::App::close(); return;
			}
			_buffer[target_entry_index]->ready = true;
		}

		void Reorder_Buffer::set_speculation_evaluation(u64 target, bool was_misprediction, time_t flush_boundary) {
			if (target >= Constants::REORDER_BUFFER_SIZE) {
				std::cout << "Tried to access non-existing reorder buffer entry.\n";
				Visualizer::App::close(); return;
			}
			_buffer[target]->ready = true;
			if (auto* branch_entry = dynamic_cast<Branch_Conditional_Reorder_Buffer_Entry*>(_buffer[target].get())) {
				branch_entry->mispredicted = was_misprediction;
				branch_entry->flush_timestamp_boundary = flush_boundary;
			}
			else if (auto* unconditional_branch_entry = dynamic_cast<Branch_Unconditional_Reorder_Buffer_Entry*>(_buffer[target].get())) {
				unconditional_branch_entry->flush_timestamp_boundary = flush_boundary;
				unconditional_branch_entry->mispredicted = was_misprediction;
			}
			else if (auto* load_entry = dynamic_cast<Load_Reorder_Buffer_Entry*>(_buffer[target].get())) {
				load_entry->flush_timestamp_boundary = flush_boundary;
				load_entry->misspeculated = was_misprediction;
			}
			else {
				std::cout << "Tried to access branch reorder buffer with wrong index.\n";
				Visualizer::App::close(); return;
			}

		}
		bool Reorder_Buffer::head_moved() {
			return _head_moved;
		}

		void Reorder_Buffer::reset() {
			_head = 0;
			_tail = 0;
			_head_moved = false;
			_flushed = false;

			for (auto& element : _buffer) {
				if (element) {
					element.reset();
				}
			}
		}


		bool Reorder_Buffer::empty() {
			return _head == _tail;
		}
		bool Reorder_Buffer::flushed() {
			return _flushed;
		}
		std::array<std::unique_ptr<Reorder_Buffer_Entry>, Constants::REORDER_BUFFER_SIZE>& Reorder_Buffer::buffer() {
			return _buffer;
		}

		bool Reorder_Buffer::full() {
			return (_tail + 1) % Constants::REORDER_BUFFER_SIZE == _head;
		}


		size_t Reorder_Buffer::head() {
			return _head;
		}
		size_t Reorder_Buffer::tail() {
			return _tail;
		}
	} // namespace Core
} // namespace OoOVis
