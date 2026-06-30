/*
 * SPDX-FileCopyrightText: 2026 f3rhd 
 *
 * SPDX-License-Identifier: MIT
 */

#include <Core/Commit/ReorderBufferEntry.h>
#include <Core/Constants/Constants.h>
#include <Core/Types/Types.h>
#include <memory>
#include <array>
namespace OoOVisual
{
	namespace Core
	{
		class Reorder_Buffer {

		public:
			static size_t												allocate(std::unique_ptr<Reorder_Buffer_Entry>&& entry);
			static void													set_ready(u64 target_entry_index);
			static void													set_speculation_evaluation(u64 target, bool was_misprediction, time_t flush_boundary);
			static void													commit();
			static std::array<std::unique_ptr<Reorder_Buffer_Entry>, Constants::REORDER_BUFFER_SIZE>& buffer();
			static bool													full();
			static size_t												head();
			static size_t												tail();
			static bool													flushed(); // Visualizer uses this
			static bool													head_moved(); // Visualizer uses this
			static void													reset();
			static bool													empty();
		private:
			static std::array<std::unique_ptr<Reorder_Buffer_Entry>, Constants::REORDER_BUFFER_SIZE>  _buffer;
			// points to the slot that is gonna be retired (if ready) next cycle
			static size_t																			  _head;
			// points to the slot that is going to be allocated 
			static size_t																			  _tail;
			// visualizer uses this
			static bool																				  _head_moved;
			// visualizer uses this
			static bool																				  _flushed;
		};
	} // namespace Core
} // namespace OoOVis