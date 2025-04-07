#pragma once

#include "tasks/pawn-wait.h"

namespace eks
{
	class PawnSleepState : public State<Pawn>
	{
	public:
		PawnSleepState()
		{
			_wait_task = create_task<PawnWaitStateTask>( 2.0f, /* random_deviation */ 1.0f );
		}

		void on_begin() override 
		{
			// NOTE: It isn't great architecture but hey, gotta do the work :(
			Pawn* owner = machine->owner;
			owner->is_sleeping = true;
		};
		
		void on_end() override 
		{
			Pawn* owner = machine->owner;
			owner->is_sleeping = false;
		};

		bool can_switch_to() const override
		{
			const Pawn* owner = machine->owner;
			const World* world = owner->get_world();
			if ( world->is_within_world_time( 6.0f, 22.0f ) ) return false;

			return true;
		}

		bool can_switch_from() const override
		{
			// Only switch when finished waiting.
			return _wait_task->is_finished();
		}

		std::string get_name() const override
		{
			return "PawnSleepState";
		}

	private:
		PawnWaitStateTask* _wait_task = nullptr;
	};
}