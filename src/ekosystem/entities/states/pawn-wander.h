#pragma once

#include "tasks/pawn-find-wander.h"
#include "tasks/pawn-move.h"
#include "tasks/pawn-wait.h"

namespace eks
{
	class PawnWanderState : public State<Pawn>
	{
	public:
		PawnWanderState()
		{
			create_task<PawnFindWanderStateTask>( &_location );
			create_task<PawnMoveStateTask>( &_location );
			create_task<PawnWaitStateTask>( 3.0f, 1.5f );
		}

		bool can_switch_to() const override
		{
			Pawn* owner = machine->owner;
			if ( owner->data->move_speed <= 0.0f ) return false;

			//	A moveable pawn can always wander
			return true;
		}

		std::string get_name() const override
		{
			return "PawnWanderState";
		}

	private:
		Vec3 _location = Vec3::zero;
	};
}