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

	private:
		Vec3 _location = Vec3::zero;
	};
}