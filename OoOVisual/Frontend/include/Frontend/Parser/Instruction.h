/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Core/Types/Types.h>
namespace OoOVisual
{
	namespace FrontEnd
	{
		auto constexpr NO_LABEL = 0;
		auto constexpr FORWARD_ADDR = 0xFFFFFFFFu;
		enum class FLOW_TYPE : u8 {
			REGISTER,
			LOAD,
			STORE,
			BRANCH_CONDITIONAL,
			BRANCH_UNCONDITIONAL,
			UNKNOWN
		};
		struct Instruction {
			virtual memory_addr_t target_addr() const { return NO_LABEL; }
			virtual bool is_label_instruction() const { return false; }
			virtual ~Instruction() = default;
			virtual FLOW_TYPE flow() const = 0;
		};
		struct Memory_Instruction : Instruction {
			Memory_Instruction(
				const reg_id_t  dest_reg_,
				const reg_id_t  base_reg_,
				offset_t offset_
			) :
				_dest_src_reg(dest_reg_),
				_base_reg(base_reg_),
				_offset(offset_) {}
		protected:
			reg_id_t _dest_src_reg;
			reg_id_t _base_reg;
			offset_t _offset;
		};
		struct Load_Instruction : Memory_Instruction {
			enum class LOAD_INSTRUCTION_TYPE {
				LB,
				LH,
				LW,
				LBU,
				LHU,
				UNKNOWN
			};
			Load_Instruction(
				LOAD_INSTRUCTION_TYPE type_,
				const reg_id_t dest_reg_,
				offset_t offset_,
				const reg_id_t base_reg_
			) :
				Memory_Instruction(dest_reg_, base_reg_, offset_), _type(type_) {}
			FLOW_TYPE flow() const override {
				return FLOW_TYPE::LOAD;
			};
			LOAD_INSTRUCTION_TYPE kind() const { return _type; }
			offset_t offset() const { return _offset; }
			reg_id_t base_reg() const { return _base_reg; };
			reg_id_t dest_reg() const { return _dest_src_reg; }
		private:
			LOAD_INSTRUCTION_TYPE _type;
		};
		struct Store_Instruction : Memory_Instruction {
			enum class STORE_INSTRUCTION_TYPE {
				SB,
				SH,
				SW,
				UNKNOWN
			};
			Store_Instruction(
				STORE_INSTRUCTION_TYPE type_,
				const reg_id_t dest_reg_,
				offset_t offset_,
				const reg_id_t base_reg_
			) :
				Memory_Instruction(dest_reg_, base_reg_, offset_), _type(type_) {}
			FLOW_TYPE flow() const override {
				return FLOW_TYPE::STORE;
			};
			offset_t offset() const { return _offset; }
			reg_id_t src1() const { return _base_reg; };
			reg_id_t src2() const { return _dest_src_reg; }
			STORE_INSTRUCTION_TYPE kind() const { return _type; };
		private:
			STORE_INSTRUCTION_TYPE _type;
		};
		struct Register_Instruction : Instruction {
			enum class REGISTER_INSTRUCTION_TYPE {
				ADD,
				SUB,
				SLL,
				SLT,
				SLTU,
				XOR,
				SRL,
				SRA,
				OR,
				AND,
				MUL,
				MULH,
				MULHSU,
				MULHU,
				DIV,
				DIVU,
				REM,
				REMU,
				LOAD_UPPER,
				AUIPC,
				UNKNOWN
			};
			union _src2_ {
				reg_id_t src2_reg;
				int32_t imm_val;
			};
			Register_Instruction(
				REGISTER_INSTRUCTION_TYPE type_,
				const reg_id_t dest_reg_,
				const reg_id_t src1_reg_,
				int32_t src2_val,
				bool is_imm_
			) :
				_dest_reg(dest_reg_),
				_src1_reg(src1_reg_),
				_type(type_),
				_is_imm(is_imm_) {

				if (_is_imm)
					_src2.imm_val = src2_val;
				else
					_src2.src2_reg = (reg_id_t)src2_val;

			}
			FLOW_TYPE flow() const override { return FLOW_TYPE::REGISTER; }
			REGISTER_INSTRUCTION_TYPE instruction_type() const { return _type; }
			bool uses_immval() const { return _is_imm; }
			reg_id_t dest_reg() const { return _dest_reg; }
			reg_id_t src1_reg() const { return _src1_reg; }
			_src2_   src2() const { return _src2; }
		private:
			_src2_ _src2;
			reg_id_t _dest_reg;
			reg_id_t _src1_reg;
			REGISTER_INSTRUCTION_TYPE _type;
			bool _is_imm = false;
		};
		struct Branch_Instruction : Instruction {
			enum class BRANCH_INSTRUCTION_TYPE {
				BEQ,
				BNE,
				BLT,
				BGE,
				BLTU,
				BGEU,
				UNKNOWN
			} _type;
			Branch_Instruction(
				BRANCH_INSTRUCTION_TYPE type_,
				reg_id_t src1_,
				reg_id_t src2_,
				memory_addr_t target_addr_
			) :
				_src1_reg(src1_),
				_src2_reg(src2_),
				_target_addr(target_addr_),
				_type(type_) {}
			memory_addr_t target_addr() const override { return _target_addr; }
			void set_target_addr(memory_addr_t label_id) { _target_addr = label_id; };
			FLOW_TYPE flow() const override { return FLOW_TYPE::BRANCH_CONDITIONAL; }
			BRANCH_INSTRUCTION_TYPE kind() const { return _type; }
			reg_id_t src1() const { return _src1_reg; }
			reg_id_t src2() const { return _src2_reg; }
		private:
			reg_id_t _src1_reg;
			reg_id_t _src2_reg;
			memory_addr_t _target_addr;
		};
		struct Jump_Instruction : Instruction {
			enum class JUMP_INSTRUCTION_TYPE {
				JALR,
				JAL,
				UNKNOWN
			} _type;
			Jump_Instruction(
				JUMP_INSTRUCTION_TYPE type_,
				reg_id_t dest_reg_,
				reg_id_t src1_,
				memory_addr_t target_addr,
				i32 imm_
			) : _type(type_),
				_dest_reg(dest_reg_),
				_src1(src1_),
				_target_addr(target_addr),
				_imm(imm_) {}
			memory_addr_t target_addr() const override {
				return _target_addr;
			}
			void set_target_addr(memory_addr_t target_addr) {
				_target_addr = target_addr;
			};
			FLOW_TYPE flow() const override {
				return FLOW_TYPE::BRANCH_UNCONDITIONAL;
			}
			JUMP_INSTRUCTION_TYPE kind() const {
				return _type;
			}
			reg_id_t src1() const { return _src1; }
			i32		 offset() const { return _imm; }
			reg_id_t dest_reg() const { return _dest_reg; }
		private:
			reg_id_t _dest_reg;
			memory_addr_t _target_addr; // jal uses this
			reg_id_t _src1;// jalr uses this
			int32_t _imm;  // jalr uses this
		};

	} // namespace Instruction
} //namespace OoOVis
