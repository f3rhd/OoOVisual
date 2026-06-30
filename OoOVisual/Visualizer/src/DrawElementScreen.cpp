// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/DrawElementScreen.h>
#include <Core/MMIO/ScreenMMIO.h>
#include <imgui_impl_opengl3_loader.h>
namespace OoOVisual
{
	namespace Visualizer
	{
		Draw_Element_Screen::Draw_Element_Screen(
				DRAW_ELEMENT_ID id,
				const ImVec2& position,
				const ImVec2& dimension
		) : Draw_Element_Core_Unit(id, position, dimension) {
			glGenTextures(1, &_texture_id);
			glBindTexture(GL_TEXTURE_2D, _texture_id);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)dimension.x, (int)dimension.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
		void Draw_Element_Screen::draw(const Camera& cam) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();


			ImVec2 world_top_left = _position;
			ImVec2 world_bottom_right = ImVec2(_position.x + _dimension.x, _position.y + _dimension.y);

			ImVec2 screen_min = cam.world_to_screen(world_top_left);
			ImVec2 screen_max = cam.world_to_screen(world_bottom_right);

			ImVec2 win_min = ImGui::GetWindowPos();
			ImVec2 win_max = ImVec2(win_min.x + ImGui::GetWindowSize().x, win_min.y + ImGui::GetWindowSize().y);

			if (screen_max.x < win_min.x || screen_min.x > win_max.x ||
				screen_max.y < win_min.y || screen_min.y > win_max.y) {
				return;
			}

			glBindTexture(GL_TEXTURE_2D, _texture_id);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexSubImage2D(
				GL_TEXTURE_2D, 0, 0, 0,
				Core::Constants::CORE_SCREEN_WIDTH,
				Core::Constants::CORE_SCREEN_HEIGHT,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				Core::Screen_MMIO::buffer().data()
			);
			draw_list->AddImage(
				(ImTextureID)(intptr_t)_texture_id,
				screen_min,
				screen_max,
				ImVec2(0, 0),
				ImVec2(1, 1)
			);
		}
		Draw_Element_Screen::~Draw_Element_Screen() {
			if (_texture_id != 0) {
				glDeleteTextures(1, &_texture_id);
			}
		}
	}
}

