// SPDX-FileCopyrightText: 2026 f3rhd 
//
// SPDX-License-Identifier: MIT

#include <Core/Core.h>
#include <Frontend/Parser/Parser.h>
#include <Core/Dispatch/Dispatcher.h>
#include <Core/Execution/ExecutionStage.h>
#include <Core/Commit/ReorderBuffer.h>
#include <Core/Fetch/Fetch.h>
#include <Core/DCache/DCache.h>
#include <Core/RegisterManager/RegisterManager.h>
#include <Core/ReservationStation/ReservationStationPool.h>
#include <Core/Constants/Constants.h>
#include <Core/MMIO/ScreenMMIO.h>
#include <Core/Execution/ExecutionUnits.h>
#include <Core/Fetch/FetchElements.h>

#include <chrono>
#include <utility>

namespace OoOVisual
{
	namespace Core
	{

		static float _tick_speed{};
		static CORE_MODE _mode{ CORE_MODE::STOP };
		static double _accumulator{};
		bool init(int argc, char** argv) {
			FrontEnd::Parser parser;
			auto cli_args(parser.parse_cli(argc, argv));
			auto parse_result(parser.parse_instructions(cli_args.input_file));
			if (!parser.was_successful())
				return false;
			Fetch_Unit::init(std::move(parse_result.first), std::move(parse_result.second));
			Register_Manager::init();
			return true;
		}
		static void tick() {
			Reorder_Buffer::commit();
			Execution_Stage::issue_and_execute();
			auto feedback(Dispatcher::dispatch_fetch_group());
			Fetch_Group::group = (Fetch_Unit::fetch(feedback));
		}

		float* tick_speed() {
			return &_tick_speed;
		}
		void run() {
			static auto _last_time{ std::chrono::high_resolution_clock::now() };
			auto now{ std::chrono::high_resolution_clock::now() };
			std::chrono::duration<double> delta{ now - _last_time };
			_last_time = now;
			switch (_mode) {
			case OoOVisual::Core::CORE_MODE::RUN:
				_accumulator += _tick_speed * Constants::MAX_IPS * delta.count();
				while (_accumulator >= 1.0f) {
					tick();
					_accumulator -= 1.0f;
				}
				break;
			case OoOVisual::Core::CORE_MODE::STEP:
				tick();
				_mode = CORE_MODE::STOP;
				break;
			case OoOVisual::Core::CORE_MODE::STOP:
			default:
				break;
			}

		}

		void reset() {

			Reorder_Buffer::reset();
			DCache::reset();
			Fetch_Unit::reset();
			Reservation_Station_Pool::reset();
			Dispatcher::reset();
			Execution_Unit_Load_Store::reset();
			Execution_Stage::reset();
			Register_Manager::reset();
			Screen_MMIO::clear_screen();
		}

		void set_core_mode(CORE_MODE mode) {
			_mode = mode;
		}

	}
}


