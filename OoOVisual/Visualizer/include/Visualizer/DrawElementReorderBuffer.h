/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#include <Visualizer/DrawElementCoreUnit.h>
#include <Core/Commit/ReorderBufferEntry.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		struct Draw_Element_Reorder_Buffer : Draw_Element_Core_Unit {
			Draw_Element_Reorder_Buffer(
				DRAW_ELEMENT_ID id,
				const ImVec2& position,
				const ImVec2& dimension
			) : Draw_Element_Core_Unit(id, position, dimension) {}
			void show_tooltip() const override;
			void show_detailed() const override;
			void draw(const Camera& cam) override;
		private:
			void render_entry_details(Core::Reorder_Buffer_Entry* entry) const;
		};
	} // namespace Visualizer


} // namespace OoOVisual
