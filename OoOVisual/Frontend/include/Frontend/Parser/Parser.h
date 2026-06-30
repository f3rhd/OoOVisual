/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>

#include <Frontend/Parser/Instruction.h>
#include <Frontend/Parser/Token.h>

namespace OoOVisual
{
	namespace FrontEnd
	{

		auto constexpr INVALID_REG_ID = 255;
		struct cli_args_t {
			std::string input_file;
		};
		class Parser {
		public:
			Parser() = default;
			std::pair < std::vector<std::unique_ptr<Instruction>>, std::vector<std::pair<std::string, size_t>>> parse_instructions(const std::string& src);
			cli_args_t  parse_cli(int argc, char** argv);
			bool        was_successful() { return _successful_parse; }
		private:
			void        parse_instruction();
			void        parse_pseudo_instruction();
			void        parse_mem_instruction();
			void        parse_alu_instruction();
			void        parse_branch_instruction();
			void        parse_jump_instruction();
			void        parse_upperimm_instruction();
			void        parse_label();
			std::string tokenize_line_text(const std::string& line_raw);
			void        advance();
		private:
			Token* _current_token = nullptr;
			size_t                                                               _current_index = 0;
			std::unordered_map<std::string, memory_addr_t>                          _label_map;
			std::vector<std::pair<Branch_Instruction*, std::string>>              _unresolved_branch_instructions;// the instructions whose labels are yet to be found
			std::vector<std::pair<Jump_Instruction*, std::string>>                _unresolved_jump_instructions;// the instructions whose labels are yet to be found
			std::vector<Token>                                                   _line_tokens;
			std::vector<std::unique_ptr<Instruction>>                            _program;
			size_t                                                               _line_number = 0;
			size_t                                                               _column = 0;
			bool                                                                 _successful_parse{ true };
		};
	} // namespace Frontend
} // namespace OooVis