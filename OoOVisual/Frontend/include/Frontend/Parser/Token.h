/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
namespace OoOVisual
{
	namespace FrontEnd
	{
		enum class TOKEN_TYPE {
			LOAD_OPERATION,
			STORE_OPERATION,
			ALU_OPERATION_R,
			ALU_OPERATION_I,
			BRANCH_OPERATION,
			JUMP_OPERATION,
			LOAD_UPPER,
			AUIPC,
			PSUEDO_OPERATION,
			REGISTER,
			IMMEDIATE,
			LABEL,
			COMMA,
			LPAREN,
			RPAREN,
			IDENTIFIER,
			NEW_LINE
		};
		struct Token {
			size_t row;
			size_t column;
			TOKEN_TYPE type;
			std::string word;
			Token(std::string&& _word, const TOKEN_TYPE _type, size_t row_, size_t column_) : word(std::move(_word)), type(_type), row(row_), column(column_) {}
		};
	} // namspeace Frontend
} // namespace OoOVis
