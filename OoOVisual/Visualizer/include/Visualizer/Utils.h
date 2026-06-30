/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <imgui.h>
#include <Frontend/Parser/Instruction.h>
#include <Core/Execution/ExecutionUnitModes.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		namespace Utils
		{

			void draw_glow_rec(ImVec2 pos, ImVec2 size, ImU32 col);
			void draw_glow_line(ImVec2 p1, ImVec2 p2, ImU32 col);
			const char* flowtype_to_str(FrontEnd::FLOW_TYPE type);
			const char* execution_mode_to_str(Core::EXECUTION_UNIT_MODE mode);
		}
	}
}