// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Frontend/Parser/Lookup.h>

namespace OoOVisual
{
	namespace FrontEnd
	{
		const std::unordered_map<std::string, reg_id_t>
			Lookup::_str_to_id_registers{
				{"zero",0},{"ra",1},{"sp",2},{"gp",3},
				{"tp",4},{"t0",5},{"t1",6},{"t2",7},
				{"s0",8},{"s1",9},{"a0",10},
				{"a1",11}, {"a2",12}, {"a3",13}, {"a4",14},
				{"a5",15}, {"a6",16}, {"a7",17}, {"s2",18},
				{"s3",19}, {"s4",20}, {"s5",21}, {"s6",22},
				{"s7",23},{"s8",24},{"s9",25}, {"s10",26},
				{"s11",27},{"t3",28}, {"t4",29}, {"t5",30},{"t6",31},{"fp",8},
				{"x0",0},{"x1",1},{"x2",2},{"x3",3},
				{"x4",4},{"x5",5},{"x6",6},{"x7",7},
				{"x8",8},{"x9",9},{"x10",10},
				{"x11",11}, {"x12",12}, {"x13",13}, {"x14",14},
				{"x15",15}, {"x16",16}, {"x17",17}, {"x18",18},
				{"x19",19}, {"x20",20}, {"x21",21}, {"x22",22},
				{"x23",23},{"x24",24},{"x25",25}, {"x26",26},
				{"x27",27},{"x28",28}, {"x29",29}, {"x30",30},{"x31",31}
		};
		const std::unordered_map<reg_id_t, std::string>
			Lookup::_id_to_str_registers{
				{0, "zero"}, {1, "ra"}, {2, "sp"}, {3, "gp"}, {4, "tp"},
				{5, "t0"}, {6, "t1"}, {7, "t2"}, {8, "s0"}, {9, "s1"},
				{10, "a0"}, {11, "a1"}, {12, "a2"}, {13, "a3"}, {14, "a4"},
				{15, "a5"}, {16, "a6"}, {17, "a7"}, {18, "s2"}, {19, "s3"},
				{20, "s4"}, {21, "s5"}, {22, "s6"}, {23, "s7"}, {24, "s8"},
				{25, "s9"}, {26, "s10"}, {27, "s11"}, {28, "t3"}, {29, "t4"},
				{30, "t5"}, {31, "t6"}

		};
		const std::unordered_map<std::string, Load_Instruction::LOAD_INSTRUCTION_TYPE>
			Lookup::_load_instructions{
				{"lw", Load_Instruction::LOAD_INSTRUCTION_TYPE::LW},
				{"lhu", Load_Instruction::LOAD_INSTRUCTION_TYPE::LHU},
				{"lh", Load_Instruction::LOAD_INSTRUCTION_TYPE::LH},
				{"lbu", Load_Instruction::LOAD_INSTRUCTION_TYPE::LBU},
				{"lb", Load_Instruction::LOAD_INSTRUCTION_TYPE::LB}
		};
		const std::unordered_map<std::string, Store_Instruction::STORE_INSTRUCTION_TYPE>
			Lookup::_store_instructions{
				{"sb", Store_Instruction::STORE_INSTRUCTION_TYPE::SB},
				{"sw", Store_Instruction::STORE_INSTRUCTION_TYPE::SW},
				{"sh", Store_Instruction::STORE_INSTRUCTION_TYPE::SH}
		};
		const std::unordered_map<std::string, Register_Instruction::REGISTER_INSTRUCTION_TYPE>
			Lookup::_alui_instructions{
				{"addi", Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD},
				{"slli", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLL},
				{"slti", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT},
				{"sltiu", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLTU},
				{"xori", Register_Instruction::REGISTER_INSTRUCTION_TYPE::XOR},
				{"srli", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRL},
				{"srai", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRA},
				{"ori",  Register_Instruction::REGISTER_INSTRUCTION_TYPE::OR},
				{"andi",Register_Instruction::REGISTER_INSTRUCTION_TYPE::AND}
		};

		const std::unordered_map<std::string, Register_Instruction::REGISTER_INSTRUCTION_TYPE>
			Lookup::_alur_instructions{
				{"add", Register_Instruction::REGISTER_INSTRUCTION_TYPE::ADD},
				{"sub", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SUB},
				{"sll", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLL},
				{"slt", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLT},
				{"sltu", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SLTU},
				{"xor", Register_Instruction::REGISTER_INSTRUCTION_TYPE::XOR},
				{"srl", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRL},
				{"sra", Register_Instruction::REGISTER_INSTRUCTION_TYPE::SRA},
				{"or",  Register_Instruction::REGISTER_INSTRUCTION_TYPE::OR},
				{"and",Register_Instruction::REGISTER_INSTRUCTION_TYPE::AND},
				{"mul",Register_Instruction::REGISTER_INSTRUCTION_TYPE::MUL},
				{"mulh",Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULH},
				{"mulhsu",Register_Instruction::REGISTER_INSTRUCTION_TYPE::MULHSU},
				{"div",Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIV},
				{"divu",Register_Instruction::REGISTER_INSTRUCTION_TYPE::DIVU},
				{"rem",Register_Instruction::REGISTER_INSTRUCTION_TYPE::REM},
				{"remu",Register_Instruction::REGISTER_INSTRUCTION_TYPE::REMU}
		};
		const std::unordered_map<std::string, Branch_Instruction::BRANCH_INSTRUCTION_TYPE>
			Lookup::_branch_instructions{
				{"beq",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BEQ},
				{"bne",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BNE},
				{"blt",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLT},
				{"bge",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGE},
				{"bltu",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BLTU},
				{"bgeu",Branch_Instruction::BRANCH_INSTRUCTION_TYPE::BGEU}
		};
		const std::unordered_map<std::string, Jump_Instruction::JUMP_INSTRUCTION_TYPE>
			Lookup::_jump_instructions{
				{"jalr",Jump_Instruction::JUMP_INSTRUCTION_TYPE::JALR},
				{"jal",Jump_Instruction::JUMP_INSTRUCTION_TYPE::JAL}
		};
		reg_id_t Lookup::reg_id(const std::string& reg_string_id) {
			if (_str_to_id_registers.find(reg_string_id) == _str_to_id_registers.end())
				return 255;
			return _str_to_id_registers.at(reg_string_id);
		}

		std::string Lookup::reg_name(const reg_id_t reg_id) {
			if (_id_to_str_registers.find(reg_id) == _id_to_str_registers.end())
				return "zero";
			return _id_to_str_registers.at(reg_id);
		}

		const std::unordered_set<std::string>
			Lookup::_pseudo_ops{
				"nop","li","mv","not","neg",
				"seqz","snez","sltz","sgtz","beqz",
				"bnez","blez","bgez","bltz","bgtz",
				"ble","bgt","bleu","bgtu","j","jal",
				"jr","jalr","ret","call","la"
		};
		Load_Instruction::LOAD_INSTRUCTION_TYPE
			Lookup::load_type(const std::string& str) {
			if (_load_instructions.find(str) == _load_instructions.end())
				return Load_Instruction::LOAD_INSTRUCTION_TYPE::UNKNOWN;
			return _load_instructions.at(str);
		}

		Store_Instruction::STORE_INSTRUCTION_TYPE
			Lookup::store_type(const std::string& str) {
			if (_store_instructions.find(str) == _store_instructions.end())
				return Store_Instruction::STORE_INSTRUCTION_TYPE::UNKNOWN;
			return _store_instructions.at(str);
		}

		Register_Instruction::REGISTER_INSTRUCTION_TYPE
			Lookup::alui_type(const std::string& str) {
			if (_alui_instructions.find(str) == _alui_instructions.end())
				return Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN;
			return _alui_instructions.at(str);
		}

		Register_Instruction::REGISTER_INSTRUCTION_TYPE
			Lookup::alur_type(const std::string& str) {
			if (_alur_instructions.find(str) == _alur_instructions.end())
				return Register_Instruction::REGISTER_INSTRUCTION_TYPE::UNKNOWN;
			return _alur_instructions.at(str);
		}

		Branch_Instruction::BRANCH_INSTRUCTION_TYPE
			Lookup::branch_type(const std::string& str) {
			if (_branch_instructions.find(str) == _branch_instructions.end())
				return Branch_Instruction::BRANCH_INSTRUCTION_TYPE::UNKNOWN;
			return _branch_instructions.at(str);
		}

		Jump_Instruction::JUMP_INSTRUCTION_TYPE
			Lookup::jump_type(const std::string& str) {
			if (_jump_instructions.find(str) == _jump_instructions.end())
				return Jump_Instruction::JUMP_INSTRUCTION_TYPE::UNKNOWN;
			return _jump_instructions.at(str);
		}

		bool Lookup::is_imm(const std::string& s) {
			if (s.empty()) return false;
			size_t str_size = s.size();
			// Hexadecimal: 0x or 0X prefix
			if (str_size > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
				for (size_t i = 2; i < str_size; ++i) {
					char c = s[i];
					if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
						return false;
					}
				}
				return true;
			}
			// Decimal: optional leading '-' and digits
			size_t i{ 0 };
			if (s[0] == '-') i = 1;
			if (i == str_size) return false;
			for (; i < str_size; ++i) {
				if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
					return false;
				}
			}
			return true;
		}
		bool Lookup::is_pseudo(const std::string& str) {
			return _pseudo_ops.count(str);
		}
	} // namespace Frontend
} // namespace OoOVis