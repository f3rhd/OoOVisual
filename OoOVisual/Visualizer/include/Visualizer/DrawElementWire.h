/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#include <Visualizer/DrawElement.h>

namespace OoOVisual
{
	namespace Visualizer
	{
		struct Draw_Element_Wire : Draw_Element {
			Draw_Element_Wire(DRAW_ELEMENT_ID _id, const ImVec2& start, const ImVec2& end) :Draw_Element(_id), _start(start), _end(end) {}
			void show_tooltip() const;
			void draw(const Camera& cam) override;
			void trigger_pulse(const ImColor& color) { _pulse_color = color; _last_pulse_time = static_cast<float>(ImGui::GetTime()); }
		private:
			ImVec2 _start{};
			ImVec2 _end{};
			ImColor _pulse_color{ 1.0f,1.0f,1.0f,1.0f };
			float _last_pulse_time{ -10.0f };
		};
	}
}