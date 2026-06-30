/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <Core/Types/Types.h>
#include <Core/Execution/ExecutionUnits.h>
namespace OoOVisual
{
	namespace Core
	{
		class DCache {
		public:
			static void	  write(EXECUTION_UNIT_MODE mode, memory_addr_t addr, data_t data);
			static data_t read(EXECUTION_UNIT_MODE mode, memory_addr_t addr);
			static void		  reset();
			static const std::unordered_map<memory_addr_t, u8> cache() { return _memory; }
		private:
			static std::unordered_map<memory_addr_t, u8> _memory;
		};
	} // namespace Core
} // namepsace OoOVis