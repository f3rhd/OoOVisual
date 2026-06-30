/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Core/ReservationStation/ReservationStation.h>
#include <Core/Types/Types.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/Constants/Constants.h>
#include <array>
namespace OoOVisual
{
	namespace Core
	{
		struct Reservation_Station_Pool {

			static time_t flush_mispredicted(u32 flusher_tag, time_t flusher_timestamp);
			static void wakeup(u32 producer_tag, data_t produced_data);
			static void deallocate_entry(u32 tag);
			static Reservation_Station& get_reservation_station(RESERVATION_STATION_ID id);
			static void reset();
		private:
			static std::array<Core::Reservation_Station, Constants::RESERVATION_STATION_AMOUNT> _pool;
		};

	} // namespace Core
} // namespace OoOVis