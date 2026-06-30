// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/Camera.h>
namespace OoOVisual
{
	namespace Visualizer
	{

		ImVec2 Camera::world_to_screen(ImVec2 world_pos) const {
			return ImVec2{
				world_pos.x * zoom + translation.x,
				world_pos.y * zoom + translation.y
			};

		}

		void Camera::update() {

			float move_speed{ 5.0f };
			if (ImGui::IsKeyDown(ImGuiKey_W)) translation.y += move_speed;
			if (ImGui::IsKeyDown(ImGuiKey_S)) translation.y -= move_speed;
			if (ImGui::IsKeyDown(ImGuiKey_A)) translation.x += move_speed;
			if (ImGui::IsKeyDown(ImGuiKey_D)) translation.x -= move_speed;

			float zoom_factor{ 1.02f };
			bool zooming{ false };

			if (ImGui::IsKeyDown(ImGuiKey_Equal) || ImGui::IsKeyDown(ImGuiKey_KeypadAdd)) {
				zoom *= zoom_factor;
				zooming = true;
			}
			if (ImGui::IsKeyDown(ImGuiKey_Minus) || ImGui::IsKeyDown(ImGuiKey_KeypadSubtract)) {
				zoom /= zoom_factor;
				zooming = true;
			}

			if (zoom < 0.1f) zoom = 0.1f;
			if (zoom > 5.0f) zoom = 5.0f;

		}

	}

}