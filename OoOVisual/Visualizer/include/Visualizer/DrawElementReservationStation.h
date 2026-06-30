/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#include <Visualizer/DrawElementCoreUnit.h>
#include <Core/ReservationStation/ReservationStation.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		struct Draw_Element_Reservation_Station : Draw_Element_Core_Unit {

			explicit Draw_Element_Reservation_Station(DRAW_ELEMENT_ID id, const ImVec2& position, const ImVec2& dimension);
			void show_tooltip() const override;
			void show_detailed() const override {};
			void draw(const Camera& cam) override;

		private:
			const Core::Reservation_Station* _core_station{};
		};

	}
}