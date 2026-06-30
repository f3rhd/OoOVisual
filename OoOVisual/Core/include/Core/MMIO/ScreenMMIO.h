/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <Core/Constants/Constants.h>
#include <Core/Types/Types.h>

#include <array>

namespace OoOVisual
{
	namespace Core
	{
		struct Screen_MMIO {
			enum class CMD_TYPE : i32 {
				UNKNOWN = 0,
				FILL_RECT = 1,
				LINE = 2,
				RECT = 3,
				TRIANGLE = 4,
				FILL_TRIANGLE = 5
			};
			static bool handle_write(u32 address, i32 value);
			static void clear_screen();
			static const std::array<u32, Constants::CORE_SCREEN_SIZE>& buffer();
		private:
			static void hollow_triangle();
			static void fill_triangle();
			static void draw_line(
				i32 start_x, i32 start_y,
				i32 end_x, i32 end_y
			);
		private:
			static CMD_TYPE _command_type;
			static i32		_param_x1;
			static i32		_param_y1;
			static i32		_param_x2;
			static i32		_param_y2;
			static i32      _param_x3;
			static i32		_param_y3;
			static i32		_param_color;
			static std::array<u32, Constants::CORE_SCREEN_SIZE> _buffer;
		};


	} // namespace Core

} // namespace OoOVisual