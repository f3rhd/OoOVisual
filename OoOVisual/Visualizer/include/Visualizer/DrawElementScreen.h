/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <Visualizer/DrawElementCoreUnit.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		struct Draw_Element_Screen : Draw_Element_Core_Unit {
			Draw_Element_Screen(
				DRAW_ELEMENT_ID id,
				const ImVec2& position,
				const ImVec2& dimension
			);
			~Draw_Element_Screen();
			void show_tooltip() const override {};
			void show_detailed() const override {};
			void draw(const Camera& cam) override;
		private:
			unsigned int _texture_id{ 0 };
		};

	}  // namespace Visualizer
} // namespace OoOVisual