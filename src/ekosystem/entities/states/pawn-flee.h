#pragma once

#include "tasks/pawn-find-food.h"
#include "tasks/pawn-move.h"
#include "tasks/pawn-eat.h"
#include "tasks/pawn-wait.h"

namespace eks
{
	class PawnFleeState : public State<Pawn>
	{
	public:
		PawnFleeState()
		{
			create_task<PawnMoveStateTask>( &_target );
		}

		void on_begin() override
		{
			//_target = 
		}

		bool can_switch_to() const override
		{
			Pawn* owner = machine->owner;
			if ( owner->data->move_speed <= 0.0f ) return false;
			// TODO: Flee condition
			//if ( !owner->flee_target.is_valid() ) return false;

			return true;
		}

		std::string get_name() const override
		{
			return "PawnFleeState";
		}

	private:
		SafePtr<Pawn> _target = nullptr;
	};
}