// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/MMIO/ScreenMMIO.h>
namespace OoOVisual
{
	namespace Core
	{
		std::array<u32, Constants::CORE_SCREEN_SIZE> Screen_MMIO::_buffer{};
		Screen_MMIO::CMD_TYPE	Screen_MMIO::_command_type;
		i32		Screen_MMIO::_param_x1;
		i32		Screen_MMIO::_param_y1;
		i32		Screen_MMIO::_param_x2;
		i32		Screen_MMIO::_param_y2;
		i32		Screen_MMIO::_param_color;
		i32     Screen_MMIO::_param_x3;
		i32     Screen_MMIO::_param_y3;

		void Screen_MMIO::draw_line(
			i32 start_x, i32 start_y,
			i32 end_x, i32 end_y
		) {
			i32 dx{ std::abs(end_x - start_x) };
			i32 dy{ std::abs(end_y - start_y) };
			i32 sx{ (start_x < end_x) ? 1 : -1 };
			i32 sy{ (end_x < end_y) ? 1 : -1 };
			i32 err{ dx - dy };

			while (true) {
				_buffer[start_x + start_y * Constants::CORE_SCREEN_WIDTH] = _param_color;
				if (start_x == end_x && start_y == end_y) break;
				i32 e2{ 2 * err };
				if (e2 > -dy) { err -= dy; start_x += sx; }
				if (e2 < dx) { err += dx; start_y += sy; }
			}

		}
		void Screen_MMIO::hollow_triangle() {
			draw_line(_param_x1, _param_y1, _param_x2, _param_y2);
			draw_line(_param_x1, _param_y1, _param_x3, _param_y3);
			draw_line(_param_x2, _param_y2, _param_x3, _param_y3);
		}
		void Screen_MMIO::fill_triangle() {
			// Sort vertices by Y (y1 <= y2 <= y3)
			i32 x1{ _param_x1 }, y1{ _param_y1 };
			i32 x2{ _param_x2 }, y2{ _param_y2 };
			i32 x3{ _param_x3 }, y3{ _param_y3 };

			if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }
			if (y1 > y3) { std::swap(x1, x3); std::swap(y1, y3); }
			if (y2 > y3) { std::swap(x2, x3); std::swap(y2, y3); }

			// Lambda: fill a horizontal span on row y from xa to xb
			auto fill_span = [&](i32 y, i32 xa, i32 xb) {
				if (xa > xb) std::swap(xa, xb);
				xa = std::max(xa, 0);
				xb = std::min(xb, Constants::CORE_SCREEN_WIDTH - 1);
				if (y < 0 || y >= Constants::CORE_SCREEN_HEIGHT) return;
				i32 base = y * Constants::CORE_SCREEN_WIDTH;
				std::fill(_buffer.begin() + base + xa,
						  _buffer.begin() + base + xb + 1, _param_color);
				};

			// Interpolate x on an edge at a given y
			auto edge_x = [](i32 y, i32 ax, i32 ay, i32 bx, i32 by) -> i32 {
				if (ay == by) return ax;
				return ax + (bx - ax) * (y - ay) / (by - ay);
				};

			// Flat-bottom triangle (y1..y2, long edge y1..y3)
			for (i32 y = y1; y <= y2; ++y) {
				i32 xa = edge_x(y, x1, y1, x3, y3);
				i32 xb = edge_x(y, x1, y1, x2, y2);
				fill_span(y, xa, xb);
			}

			// Flat-top triangle (y2..y3, long edge y1..y3)
			for (i32 y = y2; y <= y3; ++y) {
				i32 xa = edge_x(y, x1, y1, x3, y3);
				i32 xb = edge_x(y, x2, y2, x3, y3);
				fill_span(y, xa, xb);
			}

		}

		const std::array<u32, Constants::CORE_SCREEN_SIZE>& Screen_MMIO::buffer() { return _buffer; }
		void Screen_MMIO::clear_screen() {
			std::fill(_buffer.begin(), _buffer.end(), 0);
		}
		bool Screen_MMIO::handle_write(u32 address, i32 value) {
			if (address == Constants::START_ADDR && value == 1) {

				switch (_command_type) {

				case CMD_TYPE::FILL_RECT:
				{
					i32 start_index{ _param_x1 + _param_y1 * Constants::CORE_SCREEN_WIDTH };
					i32 end_index{ _param_x2 + _param_y2 * Constants::CORE_SCREEN_WIDTH };
					std::fill(_buffer.begin() + start_index, _buffer.begin() + end_index, _param_color);
					return true;
				}
				case CMD_TYPE::TRIANGLE:
				{
					hollow_triangle();
					return true;
				}
				case CMD_TYPE::FILL_TRIANGLE:
				{
					fill_triangle();
					return true;
				}

				case CMD_TYPE::LINE:
				{
					draw_line(_param_x1, _param_y1, _param_x2, _param_y2);
					return true;
				}

				case CMD_TYPE::RECT:
				{
					// Top and bottom edges
					std::fill(_buffer.begin() + _param_x1 + _param_y1 * Constants::CORE_SCREEN_WIDTH,
							  _buffer.begin() + _param_x2 + _param_y1 * Constants::CORE_SCREEN_WIDTH + 1, _param_color);
					std::fill(_buffer.begin() + _param_x1 + _param_y2 * Constants::CORE_SCREEN_WIDTH,
							  _buffer.begin() + _param_x2 + _param_y2 * Constants::CORE_SCREEN_WIDTH + 1, _param_color);

					// Left and right edges
					for (i32 y = _param_y1 + 1; y < _param_y2; ++y) {
						_buffer[_param_x1 + y * Constants::CORE_SCREEN_WIDTH] = _param_color;
						_buffer[_param_x2 + y * Constants::CORE_SCREEN_WIDTH] = _param_color;
					}
					return true;
				}
				case CMD_TYPE::UNKNOWN:
				default:
					return false;

				}
			}
			switch (address) {
			case Constants::PARAM_COL_ADDR:
				value = _param_color = ((value & 0xFF) << 24) |
					((value >> 8 & 0xFF) << 16) |
					((value >> 16 & 0xFF) << 8) |
					((value >> 24 & 0xFF));
				_param_color = value;
				return true;
			case Constants::PARAM_X1_ADDR:
				_param_x1 = value;
				return true;
			case Constants::PARAM_X2_ADDR:
				_param_x2 = value;
				return true;
			case Constants::PARAM_Y1_ADDR:
				_param_y1 = value;
				return true;
			case Constants::PARAM_Y2_ADDR:
				_param_y2 = value;
				return true;
			case Constants::PARAM_X3_ADDR:
				_param_x3 = value;
				return true;
			case Constants::PARAM_Y3_ADDR:
				_param_y3 = value;
				return true;
			case Constants::CMD_TYPE_ADDR:
				_command_type = static_cast<CMD_TYPE>(value);
				return true;
			default:
				break;
			}
			return false;
		}

	}
}