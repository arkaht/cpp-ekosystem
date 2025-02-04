#pragma once

#include "tasks/pawn-find-mate.h"
#include "tasks/pawn-move.h"
#include "tasks/pawn-mate.h"

namespace eks
{
	class PawnReproductionState : public State<Pawn>
	{
	public:
		PawnReproductionState()
		{
			create_task<PawnFindMateStateTask>( &_target );
			create_task<PawnMoveStateTask>( &_target );
			create_task<PawnMateStateTask>( &_target );
		}

		bool can_switch_to() const override
		{
			const Pawn* owner = machine->owner;
			if ( owner->data->max_child_spawn_count <= 0 ) return false;
			if ( owner->hunger < owner->data->min_hunger_for_reproduction ) return false;

			return true;
		}

		std::string get_name() const override
		{
			return "PawnReproductionState";
		}

	private:
		SafePtr<Pawn> _target;
	};
}