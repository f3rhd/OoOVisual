// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <array>
#include <Core/Constants/Constants.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/ReservationStation/ReservationStation.h>
#include <Core/ReservationStation/ReservationStationEntry.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Types/Types.h>
#ifdef DEBUG_PRINTS
#include <iostream>
#include <format>
#endif
#include <algorithm>
#include <vector>
namespace OoOVisual
{
	namespace Core
	{
		std::array<Reservation_Station, Constants::RESERVATION_STATION_AMOUNT> Reservation_Station_Pool::_pool{
				RESERVATION_STATION_ID::ADD_SUB,
				RESERVATION_STATION_ID::BITWISE,
				RESERVATION_STATION_ID::SET_LESS,
				RESERVATION_STATION_ID::MULTIPLIER,
				RESERVATION_STATION_ID::DIVIDER,
				RESERVATION_STATION_ID::LOAD_STORE,
				RESERVATION_STATION_ID::BRANCH
		};

		time_t Reservation_Station_Pool::flush_mispredicted(u32 flusher_tag, time_t flusher_timestamp) {
			std::vector<time_t> flushed_entry_timestamps{};
			for (auto& station : _pool) {
				for (auto& entry : station.get()) {
					if (entry.busy && entry.timestamp > flusher_timestamp && entry.self_tag != flusher_tag) {
#ifdef DEBUG_PRINTS
						std::cout << std::format("Dealloacted Instructions[{}] timestamp : {} from ReservationStation Entry : {} due to misprediction.\n", entry.instruction_address, entry.timestamp, entry.self_tag);
#endif
						if (entry.destination_register_id != Constants::INVALID_PHYSICAL_REGISTER_ID && !entry.destination_register_id_as_ofsset) {
							Register_Manager::deallocate(entry.destination_register_id);
						}
						auto copy_tag{ entry.self_tag };
						flushed_entry_timestamps.emplace_back(entry.timestamp);
						entry = Reservation_Station_Entry{};
						entry.self_tag = copy_tag;
					}
				}
			}
			auto it(std::max_element(flushed_entry_timestamps.begin(), flushed_entry_timestamps.end()));
			if (it != flushed_entry_timestamps.end())
				return *it;
			return Constants::TIME_ZERO;
		}

		void Reservation_Station_Pool::wakeup(u32 producer_tag, data_t produced_data) {
			for (auto& station : _pool) {
				for (auto& entry : station.get()) {
					if (entry.producer_tag1 == producer_tag) {
						entry.producer_tag1 = Constants::NO_PRODUCER_TAG;
						entry.src1 = produced_data;
						entry.ready = entry.producer_tag1 == Constants::NO_PRODUCER_TAG && entry.producer_tag2 == Constants::NO_PRODUCER_TAG;
#ifdef DEBUG_PRINTS
						std::cout << Constants::BLUE << std::format("Forwarded data:{} to Instructions[{}] timestamp : {} from ReservationStationPool[{}].\n", produced_data.SIGNED, entry.instruction_address, entry.timestamp, producer_tag) << Constants::RESET;
#endif

					}
					if (entry.producer_tag2 == producer_tag) {
						entry.producer_tag2 = Constants::NO_PRODUCER_TAG;
						entry.src2 = produced_data;
						entry.ready = entry.producer_tag1 == Constants::NO_PRODUCER_TAG && entry.producer_tag2 == Constants::NO_PRODUCER_TAG;
#ifdef DEBUG_PRINTS
						std::cout << Constants::BLUE << std::format("Forwarded data:{} to Instructions[{}] timestamp : {} from ReservationStationPool[{}].\n", produced_data.SIGNED, entry.instruction_address, entry.timestamp, producer_tag) << Constants::RESET;
#endif
					}
				}
			}
		}
		void Reservation_Station_Pool::deallocate_entry(u32 tag) {
			for (auto& station : _pool) {
				for (auto& entry : station.get()) {
					if (entry.busy && entry.ready && entry.self_tag == tag) {
#ifdef DEBUG_PRINTS
						std::cout << std::format("Deallocated Instructions[{}] timestamp : {} from ReservationStation.\n", entry.instruction_address, entry.timestamp);
#endif
						auto copy_tag{ entry.self_tag };
						entry = Reservation_Station_Entry{};
						entry.self_tag = copy_tag;
						break;
					}
				}
			}
		}

		Reservation_Station& Reservation_Station_Pool::get_reservation_station(RESERVATION_STATION_ID id) {
			switch (id) {
			case OoOVisual::Core::RESERVATION_STATION_ID::ADD_SUB:
				return _pool[0];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::BITWISE:
				return _pool[1];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::SET_LESS:
				return _pool[2];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::MULTIPLIER:
				return _pool[3];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::DIVIDER:
				return _pool[4];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::LOAD_STORE:
				return _pool[5];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::BRANCH:
				return _pool[6];
				break;
			case OoOVisual::Core::RESERVATION_STATION_ID::UNKNOWN:
				break;
			default:
				break;
			}
			return _pool[0];
		}

		void Reservation_Station_Pool::reset() {
			for (auto& station : _pool) {
				for (auto& entry : station.get()) {

					auto copy_tag{ entry.self_tag };
					entry = Reservation_Station_Entry{};
					entry.self_tag = copy_tag;
				}
			}
		}

	} // namespace Core
} // namespace OoOVis
