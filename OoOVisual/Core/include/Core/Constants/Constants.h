/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
namespace OoOVisual
{
	namespace Core
	{
		namespace Constants
		{
			auto constexpr REORDER_BUFFER_SIZE{ 64 };
			auto constexpr PHYSICAL_REGISTER_FILE_SIZE{ 128 };
			auto constexpr ARCHITECTURAL_REGISTER_AMOUNT{ 32 };
			auto constexpr REGISTER_ALIAS_TABLE_SIZE{ 32 };
			auto constexpr INVALID_PHYSICAL_REGISTER_ID{ PHYSICAL_REGISTER_FILE_SIZE };
			auto constexpr RESERVATION_STATION_SIZE{ 14 };
			auto constexpr LOAD_BUFFER_SIZE{ 20 };
			auto constexpr STORE_BUFFER_SIZE{ 20 };
			auto constexpr NO_PRODUCER_TAG{ 0xFFFFFFFFu };
			auto constexpr FETCH_WIDTH{ 3 };
			auto constexpr COMMIT_WIDTH{ 3 };
			auto constexpr BRANCH_SHIFT_REGISTER_SIZE{ 8 };
			auto constexpr RESERVATION_STATION_AMOUNT{ 7 };
			auto constexpr EXECUTION_RESULT_INVALID{ 0 };
			auto constexpr UNIT_TIME{ 10ull };
			auto constexpr RESET{ "\033[0m" };
			auto constexpr RED{ "\033[31m" };
			auto constexpr GREEN{ "\033[32m" };
			auto constexpr YELLOW{ "\033[33m" };
			auto constexpr BLUE{ "\033[34m" };
			auto constexpr MAGENTA{ "\033[35m" };
			auto constexpr CYAN{ "\033[36m" };
			auto constexpr MAX_IPS{ 60 };
			auto constexpr END_OF_TIME{ 0xFFFFFFFFu };
			auto constexpr EXECUTION_RESULT_STATION_DEALLOCATE_ONLY{ 1u << 1 };
			auto constexpr NOT_STORE{ 0xFFFFFFFF };
			auto constexpr EXECUTION_RESULT_STATION_DEALLOCATE_AND_FORWARD{ 1u << 2 };
			auto constexpr EXECUTION_RESULT_FORWARD_ONLY{ 1u << 3 };
			auto constexpr NOT_BRANCH_INSTRUCTION{ 0u };
			auto constexpr PREDICTED_TAKEN{ 1u << 1 };
			auto constexpr PREDICTED_NOT_TAKEN{ 1u << 2 };
			auto constexpr LOAD_DOES_NOT_USE_FORWARD_FROM_STORE{ 0xFFFFFFFFu };
			auto constexpr EXECUTABLE_LOAD_DOES_NOT_EXIST{ 0xFFFFFFFFu };
			// remember that no instruction will ever have a timestamp 0
			auto constexpr TIME_ZERO{ 0u };
			auto constexpr CORE_SCREEN_WIDTH{ 350 };
			auto constexpr CORE_SCREEN_HEIGHT{ 260 };
			auto constexpr CORE_SCREEN_SIZE{ CORE_SCREEN_HEIGHT * CORE_SCREEN_WIDTH };
			// Command registers for screen
			auto constexpr CMD_TYPE_ADDR{ 0x0 };
			// starting x coordinate
			auto constexpr PARAM_X1_ADDR{ 0x4 };
			// starting y coordinate
			auto constexpr PARAM_Y1_ADDR{ 0x8 };
			// ending x coordinate
			auto constexpr PARAM_X2_ADDR{ 0xC };
			// ending y coordinate
			auto constexpr PARAM_Y2_ADDR{ 0x10 };
			// color
			auto constexpr PARAM_COL_ADDR{ 0x14 };
			// start signal
			auto constexpr START_ADDR{ 0x18 };
			// could be used for drawing triangles 
			auto constexpr PARAM_X3_ADDR{ 0x1C };
			auto constexpr PARAM_Y3_ADDR{ 0x20 };
		} // namespace Constants
	} // namespace Core
} // namespace OoOVis
