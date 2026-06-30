/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
namespace OoOVisual
{
	namespace Core
	{
		class Reservation_Station;
	}
	namespace FrontEnd
	{
		struct Instruction;
	}
	union data_t {
		uint32_t UNSIGNED;
		int32_t  SIGNED;
	};
	using u64 = uint64_t;
	using u32 = uint32_t;
	using i32 = int32_t;
	using i16 = int16_t;
	using i8 = int8_t;
	using u16 = uint16_t;
	using u8 = uint8_t;
	using memory_addr_t = uint32_t;
	using offset_t = int32_t;
	using reg_id_t = uint32_t;
	using time_t = u32;
	using reservation_station_pool_t = std::unordered_map<u32, Core::Reservation_Station>;
	using fetch_group_t = std::vector<std::pair<std::unique_ptr<FrontEnd::Instruction>*, memory_addr_t>>;
}
