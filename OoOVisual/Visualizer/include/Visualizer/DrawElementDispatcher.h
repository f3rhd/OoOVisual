/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Visualizer/DrawElementCoreUnit.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		struct Draw_Element_Dispatcher : Draw_Element_Core_Unit {
			explicit Draw_Element_Dispatcher(DRAW_ELEMENT_ID id, const ImVec2& position, const ImVec2& dimension) : Draw_Element_Core_Unit(id, position, dimension) {}
			void show_tooltip() const override;
			// in hardware dispatcher itself is usually just combinational logic -- consisting of logic gates so we wont show it
			// what data it sends to other units etc. will be shown on arrows aka busses
			void show_detailed() const override {};
			void draw(const Camera& cam)  override;
		};

	}
}