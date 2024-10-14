#pragma once

#include "tasks/pawn-find-food.h"
#include "tasks/pawn-move.h"
#include "tasks/pawn-eat.h"
#include "tasks/pawn-wait.h"

namespace eks
{
	class PawnChaseState : public State<Pawn>
	{
	public:
		PawnChaseState()
		{
			create_task<PawnFindFoodStateTask>( &_target );
			create_task<PawnMoveStateTask>( &_target );
			create_task<PawnEatStateTask>( &_target );
			create_task<PawnWaitStateTask>( 1.0f, 0.5f );
		}

	private:
		SafePtr<Pawn> _target = nullptr;
	};
}