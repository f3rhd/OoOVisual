/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Visualizer/DrawElement.h>
#include <Visualizer/Camera.h>
#include <vector>
#include <memory>
namespace OoOVisual
{
	namespace Visualizer
	{
		struct Scene {
			void init();
			void play();
			void clear();
		private:
			std::vector<std::unique_ptr<Draw_Element>> _units;
			Camera									  _scene_camera{};
		};

	}
}