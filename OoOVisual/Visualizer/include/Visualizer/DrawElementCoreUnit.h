/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <imgui.h>
#include <Visualizer/Camera.h>
#include <Visualizer/DrawElement.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		struct Draw_Element_Core_Unit : Draw_Element {
			virtual ~Draw_Element_Core_Unit() = default;
			virtual void show_detailed() const = 0;
			virtual void show_tooltip() const = 0;
			Draw_Element_Core_Unit(DRAW_ELEMENT_ID id, const ImVec2& position, const ImVec2& dimension) : Draw_Element(id), _position(position), _dimension(dimension) {}
			void show_architectural(const Camera& cam) const;
			bool is_hovered(const Camera& cam) const;
			void set_detailed(const Camera& cam);
			bool detailed() const { return _detailed; }
		protected:
			ImVec2 _position{};
			ImVec2 _dimension{};
			bool _detailed{ false };
		};
	}
}