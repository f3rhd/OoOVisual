// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Visualizer/Utils.h>

namespace OoOVisual
{
	namespace Visualizer
	{
		namespace Utils
		{
			void draw_glow_rec(ImVec2 pos, ImVec2 size, ImU32 col) {
				ImDrawList* draw_list = ImGui::GetWindowDrawList();

				for (int i = 1; i <= 5; i++) {
					float padding{ (float)i * 2.0f };

					ImU32 alpha_col{ (col & 0x00FFFFFF) | (0x22000000 / i) };

					draw_list->AddRectFilled(
						ImVec2(pos.x - padding, pos.y - padding),
						ImVec2(pos.x + size.x + padding, pos.y + size.y + padding),
						alpha_col,
						10.0f + padding
					);
				}
			}

			void draw_glow_line(ImVec2 p1, ImVec2 p2, ImU32 col) {
				ImDrawList* draw_list = ImGui::GetWindowDrawList();

				draw_list->AddLine(p1, p2, (col & 0x00FFFFFF) | 0x44000000, 8.0f);

				draw_list->AddLine(p1, p2, col, 2.0f);

			}

			const char* flowtype_to_str(FrontEnd::FLOW_TYPE type) {
				switch (type) {
				case OoOVisual::FrontEnd::FLOW_TYPE::REGISTER:
					return "REGISTER";
				case OoOVisual::FrontEnd::FLOW_TYPE::LOAD:
					return "LOAD";
					break;
				case OoOVisual::FrontEnd::FLOW_TYPE::STORE:
					return "STORE";
					break;
				case OoOVisual::FrontEnd::FLOW_TYPE::BRANCH_CONDITIONAL:
					return "BRANCH";
				case OoOVisual::FrontEnd::FLOW_TYPE::BRANCH_UNCONDITIONAL:
					return "JUMP";
				case OoOVisual::FrontEnd::FLOW_TYPE::UNKNOWN:
				default:
					return "UNKNOWN";
				}
			}
			const char* execution_mode_to_str(Core::EXECUTION_UNIT_MODE mode) {
				switch (mode) {
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JAL:
					return "JAL";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_UNCONDITIONAL_JALR:
					return "JALR";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN_UNSIGNED:
					return "BGEU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_OR_EQUAL_THAN:
					return "BGE";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_EQUAL:
					return "BEQ";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_NOT_EQUAL:
					return "BNE";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN:
					return "BLT";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_THAN:
					return "BGT";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_LESS_THAN_UNSIGNED:
					return "BLTU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BRANCH_CONDITIONAL_GREATER_THAN_UNSIGNED:
					return "BGTU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_WORD:
					return "LW";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE:
					return "LB";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF:
					return "LH";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_BYTE_UNSIGNED:
					return "LBU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_LOAD_HALF_UNSIGNED:
					return "LHU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_STORE_WORD:
					return "SW";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_STORE_BYTE:
					return "SB";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOAD_STORE_STORE_HALF:
					return "SH";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::ADD_SUB_UNIT_ADD:
					return "ADD";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::ADD_SUB_UNIT_SUB:
					return "SUB";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::ADD_SUB_UNIT_LOAD_UPPER:
					return "LUI";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::ADD_SUB_UNIT_AUIPC:
					return "AUIPC";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_SIGNED:
					return "DIV";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::DIVIDER_DIVIDE_UNSIGNED:
					return "DIVU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_SIGNED:
					return "REM";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::DIVIDER_REMAINDER_UNSIGNED:
					return "REMU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_SIGNED:
					return "MUL";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH:
					return "MULH";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_SIGNED_UNSIGNED:
					return "MULHSU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::MULTIPLIER_MULTIPLY_HIGH_UNSIGNED:
					return "MULHU";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BITWISE_AND:
					return "AND";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BITWISE_XOR:
					return "XOR";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::LOGICAL_XOR:
					return "LXOR";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BITWISE_SHIFT_LEFT_LOGICAL:
					return "SLL";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_LOGICAL:
					return "SRL";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::BITWISE_SHIFT_RIGHT_ARITHMETIC:
					return "SRA";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::SET_LESS_THAN:
					return "SLT";
				case OoOVisual::Core::EXECUTION_UNIT_MODE::UNKNOWN:
					return "UNKNOWN";
				default:
					return "???";
				}
			}
		}
	}
}
