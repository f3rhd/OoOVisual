/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Core/Types/Types.h>
#include <Core/Constants/Constants.h>
#include <map>
namespace OoOVisual
{
	namespace Core
	{
		struct Physical_Register_File_Entry {
			data_t data = { 0 };
			u32 producer_tag = Constants::NO_PRODUCER_TAG;
			bool allocated = false;
		};
		struct Register_Manager {
			static void														init();
			static bool														full();
			static void														restore_alias_table();
			static void														update_retirement_alias_table_with(reg_id_t, reg_id_t);
			static void														deallocate(reg_id_t physical_register_id);
			static void														write(reg_id_t physical_register_id, data_t data);
			static reg_id_t													allocate_physical_register_for(reg_id_t architectural_register_id, u32 producer_tag);
			static reg_id_t													aliasof(reg_id_t architectural_register_id);
			static Physical_Register_File_Entry								read_with_alias(reg_id_t architectural_register_id);
			static const std::map<reg_id_t, Physical_Register_File_Entry>& phyical_register_file();
			static const std::map<reg_id_t, reg_id_t>& frontend_alias_table();
			static const std::map<reg_id_t, reg_id_t>& retirement_alias_table();
			static void														reset();
		private:
			static std::map<reg_id_t, Physical_Register_File_Entry> _physical_register_file;
			static std::map<reg_id_t, reg_id_t>					  _frontend_register_alias_table;
			static std::map<reg_id_t, reg_id_t>					  _retirement_alias_table;
		};

	} // namespace Core
} // namespace OoOVis
