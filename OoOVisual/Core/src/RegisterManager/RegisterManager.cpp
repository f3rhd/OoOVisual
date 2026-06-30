// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Constants/Constants.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/Types/Types.h>
#include <Frontend/Parser/Lookup.h>
#include <Visualizer/App.h>

#include <iostream>
#include <map>
#ifdef DEBUG_PRINTS
#include <format>
#endif
namespace OoOVisual
{
	namespace Core
	{
		std::map<reg_id_t, Physical_Register_File_Entry> Register_Manager::_physical_register_file{};
		std::map<reg_id_t, reg_id_t> Register_Manager::_frontend_register_alias_table{};
		std::map<reg_id_t, reg_id_t> Register_Manager::_retirement_alias_table{};

		void Register_Manager::init() {
			for (reg_id_t i = 0; i < Constants::PHYSICAL_REGISTER_FILE_SIZE; i++) {
				_physical_register_file.emplace(i, Physical_Register_File_Entry());
				if (i >= 0 && i <= 31) {
					_physical_register_file[i].allocated = true;
				}
			}
			for (reg_id_t j = 0; j < Constants::REGISTER_ALIAS_TABLE_SIZE; j++) {
				_frontend_register_alias_table.emplace(j, j);
			}
			_retirement_alias_table = _frontend_register_alias_table;
		}

		bool Register_Manager::full() {
			for (auto const& [key, entry] : _physical_register_file) {
				if (!entry.allocated) return false;
			}
			return true;
		}


		void Register_Manager::restore_alias_table() {
			_frontend_register_alias_table = _retirement_alias_table;
#ifdef DEBUG_PRINTS
			std::cout << std::format("Register alias table was restored due to mis-prediction.\n");
#endif
		}

		void Register_Manager::update_retirement_alias_table_with(reg_id_t architectural, reg_id_t physical) {
			_retirement_alias_table.insert_or_assign(architectural, physical);
		}

		void Register_Manager::deallocate(reg_id_t physical_register_id) {
			if (physical_register_id == 0)
				return;
			if (physical_register_id == Constants::INVALID_PHYSICAL_REGISTER_ID) {
				std::cout << "Tried to deallocate invalid register.\n";
				Visualizer::App::close(); return;
			}

			if (!_physical_register_file[physical_register_id].allocated) {
				std::cout << "Tried to deallocate an already deallocated register.\n";
				Visualizer::App::close(); return;
			}
			_physical_register_file[physical_register_id].allocated = false;
			_physical_register_file[physical_register_id].producer_tag = Constants::NO_PRODUCER_TAG;
#ifdef DEBUG_PRINTS
			std::cout << Constants::GREEN << std::format("Register File deallocated P{}.\n", physical_register_id) << Constants::RESET;
#endif
		}

		void Register_Manager::write(reg_id_t physical_register_id, data_t data) {
			if (physical_register_id == Constants::INVALID_PHYSICAL_REGISTER_ID) {
				std::cout << "Tried to write to an invalid register.\n";
				Visualizer::App::close(); return;
			}
			if (physical_register_id == 0) {
				return;
			}
			_physical_register_file[physical_register_id].data = data;
			_physical_register_file[physical_register_id].producer_tag = Constants::NO_PRODUCER_TAG;
#ifdef DEBUG_PRINTS
			std::cout << Constants::CYAN << std::format(
				"PhysicalRegisterFile[{}] <- {}\n",
				physical_register_id,
				data.SIGNED
			) << Constants::RESET;
#endif
		}

		reg_id_t Register_Manager::allocate_physical_register_for(reg_id_t architectural_register_id, u32 producer_tag) {
			if (architectural_register_id == 0) {
#ifdef DEBUG_PRINTS
				std::cout << Constants::BLUE << std::format("Register alias table was updated with zero->P0 due to register allocation.\n") << Constants::RESET;
#endif
				return 0;
			}
			for (auto it{ _physical_register_file.rbegin() }; it != _physical_register_file.rend(); it++) {
				auto& entry = it->second;
				auto& key = it->first;
				if (!entry.allocated) { // zero->P0 is direct mapping no one can allocate for it except "zero" ofc
					_frontend_register_alias_table[architectural_register_id] = key;
					entry.allocated = true;
					entry.producer_tag = producer_tag;
#ifdef DEBUG_PRINTS
					std::cout << Constants::BLUE << std::format("Register alias table was updated with x{},{}->P{} due to register allocation.\n", architectural_register_id, FrontEnd::Lookup::reg_name(architectural_register_id), key) << Constants::RESET;
#endif
					return key;
				}
			}
			return Constants::INVALID_PHYSICAL_REGISTER_ID;
		}

		reg_id_t Register_Manager::aliasof(reg_id_t architectural_register_id) {
			if (architectural_register_id == Constants::INVALID_PHYSICAL_REGISTER_ID) {
				std::cout << "Tried to read invalid register.";
				Visualizer::App::close(); return Constants::INVALID_PHYSICAL_REGISTER_ID;
			}
			return _frontend_register_alias_table[architectural_register_id];
		}

		Physical_Register_File_Entry Register_Manager::read_with_alias(reg_id_t architectural_register_id) {
			if (architectural_register_id == 0) {
				return {};
			}
			return _physical_register_file[aliasof(architectural_register_id)];
		}


		const std::map<reg_id_t, Physical_Register_File_Entry>& Register_Manager::phyical_register_file() {
			return _physical_register_file;
		}

		const std::map<reg_id_t, reg_id_t>& Register_Manager::frontend_alias_table() {
			return _frontend_register_alias_table;
		}

		const std::map<reg_id_t, reg_id_t>& Register_Manager::retirement_alias_table() {
			return _retirement_alias_table;

		}

		void Register_Manager::reset() {
			_physical_register_file.clear();
			_frontend_register_alias_table.clear();
			init();
		}

	}
}
