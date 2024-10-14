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

		bool can_switch_to() const override
		{
			Pawn* owner = machine->owner;
			if ( owner->data->move_speed <= 0.0f ) return false;
			if ( owner->data->has_adjective( Adjectives::Photosynthesis ) ) return false;
			if ( owner->hunger >= owner->data->min_hunger_to_eat ) return false;

			return true;
		}

	private:
		SafePtr<Pawn> _target = nullptr;
	};
}