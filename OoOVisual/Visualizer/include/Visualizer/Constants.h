/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <imgui.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		namespace Constants
		{

			constexpr auto WHITE{ IM_COL32(255,255,255,255) };
			constexpr auto BLACK{ IM_COL32(0,0,0,255) };
			constexpr auto RED{ IM_COL32(125,125,125,255) };
			constexpr auto GREEN{ IM_COL32(0,255,0,255) };
			constexpr auto REORDER_BUFFER_HEAD_COLOR{ IM_COL32(50, 255, 50, 80) };
			constexpr auto REORDER_BUFFER_TAIL_COLOR{ IM_COL32(255, 50, 50, 80) };
			constexpr auto INITIAL_WINDOW_WIDTH{ 800 };
			constexpr auto INITIAL_WINDOW_HEIGHT{ 600 };
		}
	}
}