/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <Core/Constants/Constants.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <array>
namespace OoOVisual
{
	namespace Core
	{

		class Reservation_Station {

		public:
			Reservation_Station(RESERVATION_STATION_ID id);
			Reservation_Station_Entry* allocate_entry();
			const Reservation_Station_Entry* issue();
			std::array<Reservation_Station_Entry, Constants::RESERVATION_STATION_SIZE>& get();
			const std::array<Reservation_Station_Entry, Constants::RESERVATION_STATION_SIZE>& getc() const;
			RESERVATION_STATION_ID id() const { return _id; }
			bool full() const;
		private:
			std::array<Reservation_Station_Entry, Constants::RESERVATION_STATION_SIZE> _buffer;
			RESERVATION_STATION_ID _id;
		};

	} // namespace Core

} // namespace OoOVis
