/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#include <Visualizer/DrawElementCoreUnit.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		struct Draw_Element_Execution_Unit : Draw_Element_Core_Unit {

			explicit Draw_Element_Execution_Unit(DRAW_ELEMENT_ID id, const ImVec2& position, const ImVec2& dimension)
				:Draw_Element_Core_Unit(id, position, dimension) {}
			void show_tooltip() const override;
			void show_detailed() const override; // only called when unit is load store
			void draw(const Camera& cam)  override;
		};
	}
}