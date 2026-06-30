// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Frontend/Parser/Parser.h>
#include <Frontend/Parser/Lookup.h>
#include <fstream>
#include <iostream>
namespace OoOVisual
{
	namespace FrontEnd
	{
#define EXPECT(EXPECTED_TOKEN_TYPE)                   \
            if (_current_token->type != EXPECTED_TOKEN_TYPE)   \
            {                                                 \
                std::cout << "\033[31m" << "Error(" << _current_token->row << "," <<  _current_token->column << ")\033[0m: " << "Token " << "'" << _current_token->word << "' did not meet the expected type"  << "\n" ;               \
                _successful_parse  = false; \
                return;  \
            }                                                             \

#define FAILED_PARSE_RETURN() std::pair<std::vector<std::unique_ptr<Instruction>>, std::vector<std::pair<std::string,size_t>>>()
		std::pair<std::vector<std::unique_ptr<Instruction>>, std::vector<std::pair<std::string, size_t>>>
			Parser::parse_instructions(const std::string& src) {
			std::ifstream file(src);
			std::vector<std::pair<std::string, size_t>> instruction_strs{};
			if (!src.ends_with(".s")) {
				std::cout << "\033[31m" << "Error: \033[0m" << "File name should end with *.s.\n";
				_successful_parse = false;
				return FAILED_PARSE_RETURN();
			}
			if (!file.is_open()) {
				std::cout << "\033[31m" << "Error: \033[0m" << "File path " << src << " doesn't exist.\n";
				_successful_parse = false;
				return FAILED_PARSE_RETURN();
			}
			std::string line_raw{};
			size_t instruction_index{};
			while (std::getline(file, line_raw)) {
				_line_number++;
				std::string line_better = tokenize_line_text(line_raw);
				if (!line_better.empty()) {

					if (line_better.back() == ':')
						instruction_strs.emplace_back(std::move(line_better), -1);
					else {
						instruction_strs.emplace_back(std::move(line_better), instruction_index++);
					}
				}
				_current_index = 0;
				_current_token = &_line_tokens[0];
				parse_instruction();
			}
			if (!_successful_parse)
				return FAILED_PARSE_RETURN();
			// resolve the unresolved instructions
			for (auto& branch_instruction_data : _unresolved_branch_instructions) {
				if (_label_map.find(branch_instruction_data.second) != _label_map.end()) {
					branch_instruction_data.first->set_target_addr(_label_map.at(branch_instruction_data.second));
				}
				else {
					std::cout << "\033[31m" << "Error: \033[0m: " << "Cause: " << "'" << branch_instruction_data.second << "' Unknown label identifier was found" << "\n";               \
						_successful_parse = false;
				}
			}
			for (auto& jump_instruction_data : _unresolved_jump_instructions) {
				if (_label_map.find(jump_instruction_data.second) != _label_map.end()) {
					jump_instruction_data.first->set_target_addr(_label_map.at(jump_instruction_data.second));
				}
				else {
					std::cout << "\033[31m" << "Error: \033[0m: " << "Cause: " << "'" << jump_instruction_data.second << "' Unknown label identifier was found" << "\n";               \
						_successful_parse = false;
				}
			}
			file.close();

			return { std::move(_program),std::move(instruction_strs) };
		}

		cli_args_t Parser::parse_cli(int argc, char** argv) {
			std::string input_file{};
			std::string log_destination("none");
			bool valid = true;

			for (int i = 1; i < argc; ++i) {
				std::string arg(argv[i]);

				if (input_file.empty()) {
					input_file = arg;
				}
				else {
					std::cerr << "Warning: Multiple input files found. Using first one: " << input_file << ".\n";
				}
			}

			if (input_file.empty()) {
				std::cerr << "Usage: " << argv[0]
					<< " <input.s> \n";
				valid = false;
			}

			return { std::move(input_file) };
		}



		void Parser::parse_instruction() {
			switch (_current_token->type) {
			case TOKEN_TYPE::LOAD_OPERATION:
			case TOKEN_TYPE::STORE_OPERATION:
				parse_mem_instruction();
				break;
			case TOKEN_TYPE::ALU_OPERATION_R:
			case TOKEN_TYPE::ALU_OPERATION_I:
				parse_alu_instruction();
				break;
			case TOKEN_TYPE::BRANCH_OPERATION:
				parse_branch_instruction();
				break;
			case TOKEN_TYPE::JUMP_OPERATION:
				parse_jump_instruction();
				break;
			case TOKEN_TYPE::LOAD_UPPER:
			case TOKEN_TYPE::AUIPC:
				parse_upperimm_instruction();
				break;
			case TOKEN_TYPE::LABEL:
				parse_label();
				break;
			case TOKEN_TYPE::PSUEDO_OPERATION:
				parse_pseudo_instruction();
				break;
			default:
				break;
			}
		}

		// @call : current token is load or memory instruction
		void Parser::parse_mem_instruction() {
			Token tmp(*_current_token);
			advance();
			// we should be register token
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t dest_reg_id{ Lookup::reg_id(_current_token->word) };
			advance();
			// we should be comma
			EXPECT(TOKEN_TYPE::COMMA);

			advance();

			// we should be imm
			EXPECT(TOKEN_TYPE::IMMEDIATE);
			offset_t offset{};
			if (_current_token->word[0] == '0' && _current_token->word[1] == 'x')
				offset = std::stoul(_current_token->word, nullptr, 16);
			else
				offset = std::stoul(_current_token->word);

			advance();
			// we should be left paranthesis
			EXPECT(TOKEN_TYPE::LPAREN);

			advance();
			// we should be register
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t base_reg_id{ Lookup::reg_id(_current_token->word) };

			if (tmp.type == TOKEN_TYPE::LOAD_OPERATION) {
				Load_Instruction::LOAD_INSTRUCTION_TYPE type{ Lookup::load_type(tmp.word) };
				_program.emplace_back(std::make_unique<Load_Instruction>(
					type,
					dest_reg_id,
					offset,
					base_reg_id
				));
			}
			else if (tmp.type == TOKEN_TYPE::STORE_OPERATION) {
				Store_Instruction::STORE_INSTRUCTION_TYPE type{ Lookup::store_type(tmp.word) };
				_program.emplace_back(std::make_unique<Store_Instruction>(
					type,
					dest_reg_id,
					offset,
					base_reg_id
				));
			}
			advance();
			EXPECT(TOKEN_TYPE::RPAREN);
		}

		// @call : current token is alu operation
		void Parser::parse_alu_instruction() {
			Token tmp = *_current_token;
			bool is_imm = false;
			Register_Instruction::REGISTER_INSTRUCTION_TYPE type{};
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t dest_reg_id{ Lookup::reg_id(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t src1_reg_id{ Lookup::reg_id(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			i32 src2_val{};
			// if our operation was imm
			if (Lookup::alui_type(tmp.word) != Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN) {
				type = Lookup::alui_type(tmp.word);
				EXPECT(TOKEN_TYPE::IMMEDIATE);
				is_imm = true;
				if (_current_token->word[0] == '0' && _current_token->word[1] == 'x')
					src2_val = std::stoul(_current_token->word, nullptr, 16);
				else
					src2_val = std::stoul(_current_token->word);
			}
			else if (Lookup::alur_type(tmp.word) != Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN) {
				type = Lookup::alur_type(tmp.word);
				EXPECT(TOKEN_TYPE::REGISTER);
				src2_val = Lookup::reg_id(_current_token->word);
			}
			advance();
			_program.emplace_back(
				std::make_unique<Register_Instruction>(
					type,
					dest_reg_id,
					src1_reg_id,
					src2_val,
					is_imm
				)
			);
		}

		// @call : current token is branch instruction
		void Parser::parse_branch_instruction() {
			Branch_Instruction::BRANCH_INSTRUCTION_TYPE type{ Lookup::branch_type(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t src1_id{ Lookup::reg_id(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t src2_id{ Lookup::reg_id(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			EXPECT(TOKEN_TYPE::IDENTIFIER);
			memory_addr_t memory_addr{ FORWARD_ADDR };
			if (_label_map.find(_current_token->word) != _label_map.end()) {
				memory_addr = _label_map.at(_current_token->word);
			}
			auto branch_instruction(std::make_unique<Branch_Instruction>(
				type,
				src1_id,
				src2_id,
				memory_addr
				//unique_branch_id(),
				//static_cast<u32>(_program.size())
			)
			);
			auto branch_instruction_ptr(branch_instruction.get());
			_program.emplace_back(std::move(branch_instruction));
			// couldnt find the label identifier save it for later
			if (memory_addr == FORWARD_ADDR) {
				_unresolved_branch_instructions.emplace_back(branch_instruction_ptr, _current_token->word);
			}
		}
		// @call : current token is jump instruction
		void Parser::parse_jump_instruction() {
			memory_addr_t target_addr{ FORWARD_ADDR };
			Jump_Instruction::JUMP_INSTRUCTION_TYPE type{ Lookup::jump_type(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t dest_reg{ Lookup::reg_id(_current_token->word) };
			reg_id_t src1{ INVALID_REG_ID };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			if (type == Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL) {
				EXPECT(TOKEN_TYPE::IDENTIFIER);
				if (_label_map.find(_current_token->word) != _label_map.end()) {
					target_addr = _label_map.at(_current_token->word);
				}
				// src1 and imm fields can be garbage values in this case
				auto jump_instruction{ std::make_unique<Jump_Instruction>(
					type,
					dest_reg,
					src1,
					target_addr,
					0
				)
				};
				auto jump_instruction_ptr(jump_instruction.get());
				_program.emplace_back(
					std::move(jump_instruction)
				);
				// we werent able to find the label maybe we will next time
				if (target_addr == FORWARD_ADDR)
					_unresolved_jump_instructions.emplace_back(jump_instruction_ptr, _current_token->word);
				return;
			}
			EXPECT(TOKEN_TYPE::REGISTER);
			src1 = Lookup::reg_id(_current_token->word);
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			EXPECT(TOKEN_TYPE::IMMEDIATE);
			i32 imm{};
			if (_current_token->word[0] == '0' && _current_token->word[1] == 'x')
				imm = std::stoul(_current_token->word, nullptr, 16);
			else
				imm = std::stoul(_current_token->word);
			// here we dont care about the label_id field
			_program.emplace_back(
				std::make_unique<Jump_Instruction>(
					type,
					dest_reg,
					src1,
					target_addr,
					imm
				));
		}

		// @call : current token is lui or auipc
		void Parser::parse_upperimm_instruction() {
			Token tmp(*_current_token);
			advance();
			EXPECT(TOKEN_TYPE::REGISTER);
			reg_id_t dest_reg{ Lookup::reg_id(_current_token->word) };
			advance();
			EXPECT(TOKEN_TYPE::COMMA);
			advance();
			EXPECT(TOKEN_TYPE::IMMEDIATE);
			int32_t imm{};
			if (_current_token->word[0] == '0' && _current_token->word[1] == 'x')

				imm = std::stoul(_current_token->word, nullptr, 16);
			else
				imm = std::stoul(_current_token->word);
			if (!(static_cast<uint32_t>(imm) >= 1048576)) {
				std::cout << "\033[31m" << "Error(" << _current_token->row << "," << _current_token->column << ")\033[0m: " << "Token " << "'" << _current_token->word << "should live in 20 bit range." << "\n";               \
					_successful_parse = false;
				return;
			}
			if (tmp.word == "lui")
				_program.emplace_back(std::make_unique<Register_Instruction>(
					Register_Instruction::REGISTER_INSTRUCTION_TYPE::LOAD_UPPER,
					dest_reg,
					0,
					imm,
					true
				)
				);
			else if (tmp.word == "auipc")
				_program.emplace_back(std::make_unique<Register_Instruction>(
					Register_Instruction::REGISTER_INSTRUCTION_TYPE::AUIPC,
					dest_reg,
					0,
					imm,
					true
				)
				);
		}

		// @call : current token is label
		void Parser::parse_label() {
			// insert new entry to the label map
			while (_current_token->type == TOKEN_TYPE::LABEL) {
				_label_map.emplace(_current_token->word, static_cast<u32>(_program.size()));
				advance();
			}

		}
		void Parser::advance() {

			if (_current_token->type != TOKEN_TYPE::NEW_LINE) {
				_current_index++;
				_current_token = &_line_tokens[_current_index];
			}
		}

		std::string Parser::tokenize_line_text(const std::string& line_raw) {
			_line_tokens.clear();
			_column = 0;
			size_t comment_pos{ line_raw.find('#') };
			std::string line((comment_pos != std::string::npos) ? line_raw.substr(0, comment_pos) : line_raw);
			if (line == line_raw) {
				comment_pos = line_raw.find(';');
				line = (comment_pos != std::string::npos) ? line_raw.substr(0, comment_pos) : line_raw;
			}
			size_t i{ 0 };
			while (i < line.size()) {
				_column++;
				char ch{ line[i] };

				// Skip whitespace
				if (std::isspace(static_cast<unsigned char>(ch))) {
					++i;
					continue;
				}

				// Handle single-char _line_tokens
				if (ch == ',' || ch == '(' || ch == ')') {
					TOKEN_TYPE type{};
					if (ch == ',') type = TOKEN_TYPE::COMMA;
					else if (ch == '(') type = TOKEN_TYPE::LPAREN;
					else type = TOKEN_TYPE::RPAREN;

					_line_tokens.emplace_back(std::string(1, ch), type, _line_number, _column);
					++i;
					continue;
				}

				// Match [a-zA-Z0-9_.:-]+
				size_t start{ i };
				while (i < line.size() && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_' || line[i] == '.' || line[i] == '-' || line[i] == ':')) {
					++i;
				}

				if (start == i)
					continue;

				std::string token(line.substr(start, i - start));

				// Handle label: `label:`
				if (!token.empty() && token.back() == ':') {
					_line_tokens.emplace_back(token.substr(0, token.length() - 1), TOKEN_TYPE::LABEL, _line_number, _column);
					continue;
				}

				if (Lookup::load_type(token) != Load_Instruction::LOAD_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::LOAD_OPERATION, _line_number, _column);
				}
				else if (Lookup::store_type(token) != Store_Instruction::STORE_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::STORE_OPERATION, _line_number, _column);
				}
				else if (Lookup::alui_type(token) != Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::ALU_OPERATION_I, _line_number, _column);
				}
				else if (Lookup::alur_type(token) != Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::ALU_OPERATION_R, _line_number, _column);
				}
				else if (Lookup::branch_type(token) != Branch_Instruction::BRANCH_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::BRANCH_OPERATION, _line_number, _column);
				}
				else if (Lookup::jump_type(token) != Jump_Instruction::JUMP_INSTRUCTION_TYPE::UNKNOWN) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::JUMP_OPERATION, _line_number, _column);
				}
				else if (Lookup::is_pseudo(token)) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::PSUEDO_OPERATION, _line_number, _column);
				}
				else if (token == "lui") {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::LOAD_UPPER, _line_number, _column);
				}
				else if (token == "auipc") {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::AUIPC, _line_number, _column);
				}
				else if (Lookup::reg_id(token) != INVALID_REG_ID) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::REGISTER, _line_number, _column);
				}
				else if (Lookup::is_imm(token)) {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::IMMEDIATE, _line_number, _column);
				}
				else {
					_line_tokens.emplace_back(std::move(token), TOKEN_TYPE::IDENTIFIER, _line_number, _column);
				}
			}
			_line_tokens.emplace_back(std::string(), TOKEN_TYPE::NEW_LINE, _line_number, _column);

			if (_line_tokens[0].word == "jal" && _line_tokens.size() == 3) {
				_line_tokens[0].type = TOKEN_TYPE::PSUEDO_OPERATION;
			}
			if (_line_tokens.size() > 1)
				return line;
			else
				return std::string();
		}

		// @call : current token is pseudo instruction
		// very long function im too lazy to refactor
		void Parser::parse_pseudo_instruction() {
			if (_current_token->word == "nop") {
				advance();
				EXPECT(TOKEN_TYPE::NEW_LINE);
				_program.emplace_back(
					std::make_unique<Register_Instruction>(
						Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD,
						0,
						0,
						0,
						true
					)
				);
				return;
			}
			// ret -> jalr  x0,  ra,  0
			if (_current_token->word == "ret") {
				advance();
				EXPECT(TOKEN_TYPE::NEW_LINE);
				_program.emplace_back(
					std::make_unique<Jump_Instruction>(
						Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR,
						0,
						Lookup::reg_id("ra"),
						NO_LABEL,
						0
					)
				);
				return;
			}
			if (_current_token->word == "call") {
				advance();
				EXPECT(TOKEN_TYPE::IDENTIFIER);

				memory_addr_t target_label_id{ FORWARD_ADDR };
				if (_label_map.find(_current_token->word) != _label_map.end())
					target_label_id = _label_map[_current_token->word];

				std::unique_ptr<Jump_Instruction> jump_instruction{ std::make_unique<Jump_Instruction>(
					Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL,
					Lookup::reg_id("ra"),
					0,
					target_label_id,
					0 // noimm
				)
				};
				auto jump_instruction_ptr(jump_instruction.get());
				if (target_label_id == FORWARD_ADDR)
					_unresolved_jump_instructions.emplace_back(jump_instruction_ptr, _current_token->word);

				_program.emplace_back(std::move(jump_instruction));
				return;
			}
			// li rd, imm
			if (_current_token->word == "li") {
				advance();
				EXPECT(TOKEN_TYPE::REGISTER);

				reg_id_t dest_reg{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::COMMA);

				advance();
				EXPECT(TOKEN_TYPE::IMMEDIATE);

				int32_t imm_val{};
				if (_current_token->word[0] == '0' && _current_token->word[1] == 'x')
					imm_val = std::stoul(_current_token->word, nullptr, 16);
				else
					imm_val = std::stoul(_current_token->word);

				int32_t low{ imm_val & 0xFFF };
				int32_t high(static_cast<uint32_t>(imm_val) >> 12);

				if (static_cast<uint32_t>(imm_val) < 4096) {
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD,
							dest_reg,
							0,
							(low),
							true
						)
					);
				}
				else {
					_program.emplace_back(std::make_unique<Register_Instruction>(
						Register_Instruction::REGISTER_INSTRUCTION_TYPE::LOAD_UPPER,
						dest_reg,
						0,
						high,
						true
					)
					);
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD,
							dest_reg,
							dest_reg,
							low,
							true
						)
					);
				}
				return;
			}

			// these pseudo instructions have the same syntax
			if
				(_current_token->word == "mv" ||
					_current_token->word == "not" ||
					_current_token->word == "neg" ||
					_current_token->word == "seqz" ||
					_current_token->word == "snez" ||
					_current_token->word == "sltz" ||
					_current_token->word == "sgtz"
					) {
				std::string op{ _current_token->word };
				advance();
				EXPECT(TOKEN_TYPE::REGISTER);

				reg_id_t dest_reg{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::COMMA);

				advance();

				EXPECT(TOKEN_TYPE::REGISTER);

				reg_id_t src_reg{ Lookup::reg_id(_current_token->word) };

				//  mv   rd,  rs1 -> addi  rd,  rs1, 0
				if (op[0] == 'm') {
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD,
							dest_reg,
							src_reg,
							0,
							true
						)
					);
					return;
				}
				// not  rd,  rs1 -> xori  rd,  rs1, —1
				if (op[0] == 'n' && op[1] == 'o') {
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::XOR,
							dest_reg,
							src_reg,
							-1,
							true
						)
					);
					return;
				}
				// neg  rd,  rs1 ->  sub   rd,  x0,  rs1
				if (op[0] == 'n' && op[1] == 'e') {
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::SUB,
							dest_reg,
							0,
							src_reg,
							false
						)
					);
					return;
				}

				// seqz rd,  rs1 -> sltiu rd,  rs1, 1
				if (op[1] == 'e') {

					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLTU,
							dest_reg,
							src_reg,
							1,
							true
						)
					);
					return;
				}
				// snez rd, rs1 ->  sltu  rd,  x0,  rs1
				if (op[1] == 'n') {

					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLTU,
							dest_reg,
							0,
							src_reg,
							false
						)
					);
					return;
				}
				// sltz rd,  rs1  -> slt   rd,  rs1, x0
				if (op[1] == 'l') {
					_program.emplace_back(
						std::make_unique<Register_Instruction>(
							Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT,
							dest_reg,
							src_reg,
							0,
							false
						)
					);
					return;

				}

				// sgtz rd,  rs1 -> slt   rd,  x0,  rs1
				_program.emplace_back(
					std::make_unique<Register_Instruction>(
						Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT,
						dest_reg,
						0,
						src_reg,
						false
					)
				);
				return;
			}

			if
				(
					_current_token->word == "beqz" ||
					_current_token->word == "bnez" ||
					_current_token->word == "blez" ||
					_current_token->word == "bgez" ||
					_current_token->word == "bltz" ||
					_current_token->word == "bgtz"
					) {
				std::string op{ _current_token->word };

				advance();
				EXPECT(TOKEN_TYPE::REGISTER);

				reg_id_t src1{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::COMMA);

				advance();
				EXPECT(TOKEN_TYPE::IDENTIFIER);

				memory_addr_t target_addr{ FORWARD_ADDR };
				//branch_instruction_id_t branch_id{ unique_branch_id() };
				std::unique_ptr<Branch_Instruction> branch_instruction;
				Branch_Instruction* branch_instruction_ptr{ nullptr };
				if (_label_map.find(_current_token->word) != _label_map.end())
					target_addr = _label_map[_current_token->word];

				// beqz rs1, label  -> beq   rs1, x0,  label
				if (op[1] == 'e')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BEQ,
						src1,
						0,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);
				// bnez rs1, label ->  bne   rs1, x0,  label
				else if (op[1] == 'n')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BNE,
						src1,
						0,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);
				//   blez rs1, label -> bge   x0,  rs1, label
				else if (op[1] == 'l' && op[2] == 'e')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGE,
						0,
						src1,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);

				// bgez rs1, label -> bge   rs1, x0,  label
				else if (op[1] == 'g' && op[2] == 'e')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGE,
						src1,
						0,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);
				// bltz rs1, labe ->  blt   rs1, x0,  label
				else if (op[1] == 'l' && op[2] == 't')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLT,
						src1,
						0,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);
				// bgtz rs1, label ->  blt   x0,  rs1, label
				else if (op[1] == 'g' && op[2] == 't')
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLT,
						0,
						src1,
						target_addr
						//branch_id,
						//static_cast<u32>(_program.size())
					);
				branch_instruction_ptr = branch_instruction.get();
				if (target_addr == FORWARD_ADDR)
					_unresolved_branch_instructions.emplace_back(branch_instruction_ptr, _current_token->word);
				_program.emplace_back(std::move(branch_instruction));
				return;
			}  // pseudo_branch0
			if
				(
					_current_token->word == "ble" ||
					_current_token->word == "bgt" ||
					_current_token->word == "bleu" ||
					_current_token->word == "bgtu"
					) {

				std::string op{ _current_token->word };

				advance();
				EXPECT(TOKEN_TYPE::REGISTER);
				reg_id_t src1{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::COMMA);

				advance();
				EXPECT(TOKEN_TYPE::REGISTER);
				reg_id_t src2{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::COMMA);

				advance();
				EXPECT(TOKEN_TYPE::IDENTIFIER);

				memory_addr_t target_addr{ FORWARD_ADDR };
				//branch_instruction_id_t branch_id { unique_branch_id()};
				std::unique_ptr<Branch_Instruction> branch_instruction;

				if (_label_map.find(_current_token->word) != _label_map.end())
					target_addr = _label_map[_current_token->word];

				// ble rs1, rs2, label  ->  bge rs2, rs1, label (if rs2 >= rs1, then rs1 <= rs2)
				if (op == "ble")
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGE,
						src2, src1, target_addr /*branch_id,static_cast<u32>(_program.size()*/
					);

				// bgt rs1, rs2, label  ->  blt rs2, rs1, label (if rs2 < rs1, then rs1 > rs2)
				else if (op == "bgt")
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLT,
						src2, src1, target_addr /*branch_id,static_cast<u32>(_program.size()*/
					);

				// bleu rs1, rs2, label ->  bgeu rs2, rs1, label
				else if (op == "bleu")
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGEU,
						src2, src1, target_addr /*branch_id,static_cast<u32>(_program.size()*/
					);

				// bgtu rs1, rs2, label ->  bltu rs2, rs1, label
				else if (op == "bgtu")
					branch_instruction = std::make_unique<Branch_Instruction>(
						Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLTU,
						src2, src1, target_addr /*branch_id,static_cast<u32>(_program.size()*/
					);

				Branch_Instruction* branch_instruction_ptr = branch_instruction.get();
				if (target_addr == FORWARD_ADDR)
					_unresolved_branch_instructions.emplace_back(branch_instruction_ptr, _current_token->word);

				_program.emplace_back(std::move(branch_instruction));
				return;
			}

			if (_current_token->word == "j" || _current_token->word == "jal") {
				std::string op{ _current_token->word };
				advance();
				EXPECT(TOKEN_TYPE::IDENTIFIER);

				memory_addr_t target_addr{ FORWARD_ADDR };
				if (_label_map.find(_current_token->word) != _label_map.end())
					target_addr = _label_map[_current_token->word];

				std::unique_ptr<Jump_Instruction> jump_instruction;
				// j label -> jal   x0,  label
				if (op.size() == 1) {
					jump_instruction = std::make_unique<Jump_Instruction>(
						Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL,
						0,
						0,
						target_addr,
						0
					);
					// jal  label -> jal   ra,  label
				}
				else {
					jump_instruction = std::make_unique<Jump_Instruction>(
						Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL,
						Lookup::reg_id("ra"),
						0,
						target_addr,
						0
					);
				}
				if (target_addr == FORWARD_ADDR) {
					_unresolved_jump_instructions.emplace_back(jump_instruction.get(), _current_token->word);
				}
				_program.emplace_back(std::move(jump_instruction));
				return;
			}
			if (_current_token->word == "jr" || _current_token->word == "jalr") {
				std::string op{ _current_token->word };

				advance();

				EXPECT(TOKEN_TYPE::REGISTER);
				reg_id_t src1{ Lookup::reg_id(_current_token->word) };

				advance();
				EXPECT(TOKEN_TYPE::NEW_LINE);

				std::unique_ptr<Jump_Instruction> jump_instruction;

				// jr rs1 -> jalr  x0,  rs1, 0
				if (op.size() == 2) {
					jump_instruction = std::make_unique<Jump_Instruction>(
						Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR,
						0,
						src1,
						0,
						0
					);
				}
				// jalr rs1 ->  jalr  ra,  rs1, 0
				else {
					jump_instruction = std::make_unique<Jump_Instruction>(
						Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR,
						Lookup::reg_id("ra"),
						src1,
						0,
						0
					);
				}
				_program.emplace_back(std::move(jump_instruction));
				return;
			}
		}

	} // namespace Frontend


} // namespace OoOVis