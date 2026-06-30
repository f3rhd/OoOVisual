/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma  once
#include <Core/Types/Types.h>
#include <Core/Constants/Constants.h>
namespace OoOVisual
{
	namespace Core
	{
		/*
		instructions will generate this kind of layout upon execution and will be sent to reservation stations,
		at the same time set the respectful rob entry ready
		*/
		struct Execution_Result {
			u32			kind = Constants::EXECUTION_RESULT_INVALID;
			data_t		produced_data{};
			u32			producer_tag{};
			bool 		misprediction_was_detected{ false };
		};
	} // namespace Core
} // namespace OoOVis