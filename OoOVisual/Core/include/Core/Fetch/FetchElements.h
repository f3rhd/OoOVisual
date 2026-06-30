/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <memory>
#include <Frontend/Parser/Instruction.h>
#include <Core/Types/Types.h>
#include <Core/Constants/Constants.h>
namespace OoOVisual
{
	namespace Core
	{
		struct Fetch_Element {
			std::unique_ptr<FrontEnd::Instruction>* instruction{};
			memory_addr_t address = 0;
			u32 timestamp = 0;
			u8 branch_prediction = Constants::NOT_BRANCH_INSTRUCTION;
		};
		struct Fetch_Group {
			static std::vector<Fetch_Element> group;
		};

	}
}