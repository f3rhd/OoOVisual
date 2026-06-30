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

		struct Camera {

			Camera() = default;
			ImVec2 world_to_screen(ImVec2 world_pos) const;
			void update();
			ImVec2 translation{ 0.0f, 0.0f };
			float zoom{ 1.0f };
		};
	}
}

