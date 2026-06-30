/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once

#include <Core/Types/Types.h>
#include <Frontend/Parser/Instruction.h>
#include <Core/Constants/Constants.h>
namespace OoOVisual
{
	namespace Core
	{
		struct Reorder_Buffer_Entry {
			virtual ~Reorder_Buffer_Entry() = default;
			Reorder_Buffer_Entry(FrontEnd::FLOW_TYPE flow, time_t self_timestamp_) : flow_type(flow), self_timestamp(self_timestamp_) {}
			virtual FrontEnd::FLOW_TYPE flow() { return flow_type; };
		public:
			bool ready = false;
			time_t self_timestamp = 0;
			FrontEnd::FLOW_TYPE flow_type = FrontEnd::FLOW_TYPE::UNKNOWN;
		};
		struct Register_Reorder_Buffer_Entry : Reorder_Buffer_Entry { // load dispatching and jump will also create this type of entry
			reg_id_t old_alias;
			reg_id_t new_alias;
			reg_id_t architectural_register_id;
			Register_Reorder_Buffer_Entry(
				FrontEnd::FLOW_TYPE flow,
				reg_id_t architectural,
				reg_id_t old,
				reg_id_t new_,
				time_t self_timestamp_
			) : old_alias(old),
				new_alias(new_),
				architectural_register_id(architectural),
				Reorder_Buffer_Entry(flow, self_timestamp_) {}
		};
		struct Store_Reorder_Buffer_Entry : Reorder_Buffer_Entry {
			u32 store_id;
			Store_Reorder_Buffer_Entry(FrontEnd::FLOW_TYPE flow, u32 id_, time_t self_timestamp) : store_id(id_), Reorder_Buffer_Entry(flow, self_timestamp) {}
		};
		struct Load_Reorder_Buffer_Entry : Register_Reorder_Buffer_Entry {
			time_t flush_timestamp_boundary = Constants::TIME_ZERO;
			bool   misspeculated = false;
			Load_Reorder_Buffer_Entry(
				FrontEnd::FLOW_TYPE flow,
				reg_id_t architectural,
				reg_id_t old,
				reg_id_t new_,
				time_t self_timestamp_
			) : Register_Reorder_Buffer_Entry(flow, architectural, old, new_, self_timestamp_) {}
		};
		struct Branch_Conditional_Reorder_Buffer_Entry : Reorder_Buffer_Entry {

			bool mispredicted = false;
			time_t flush_timestamp_boundary = Constants::TIME_ZERO;
			Branch_Conditional_Reorder_Buffer_Entry(
				FrontEnd::FLOW_TYPE flow,
				time_t self_timestamp_
			) :
				Reorder_Buffer_Entry(flow, self_timestamp_) {}
		};
		struct Branch_Unconditional_Reorder_Buffer_Entry : Reorder_Buffer_Entry {
			bool mispredicted = false;
			reg_id_t old_alias;
			reg_id_t new_alias;
			reg_id_t architectural_register_id;
			time_t   flush_timestamp_boundary = Constants::TIME_ZERO;
			Branch_Unconditional_Reorder_Buffer_Entry(
				FrontEnd::FLOW_TYPE flow,
				reg_id_t architectural,
				reg_id_t old,
				reg_id_t new_,
				bool mispredicted_, // jalr will always be true 
				time_t self_timestamp_
			) : old_alias(old),
				new_alias(new_),
				architectural_register_id(architectural),
				mispredicted(mispredicted_),
				Reorder_Buffer_Entry(flow, self_timestamp_) {}

		};
	} // namespace Core
} // namespace OoOVis
