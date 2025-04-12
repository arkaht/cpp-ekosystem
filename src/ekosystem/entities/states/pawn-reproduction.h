#pragma once

#include "tasks/pawn-find-mate.h"
#include "tasks/pawn-move.h"
#include "tasks/pawn-mate.h"

namespace eks
{
	class PawnReproductionState : public State<Pawn>
	{
	public:
		PawnReproductionState( Pawn* owner )
		{
			create_task<PawnFindMateStateTask>( &owner->partner_pawn );
			create_task<PawnMoveStateTask>( &owner->partner_pawn, /* acceptance_radius */ 1.0f );
			create_task<PawnMateStateTask>( &owner->partner_pawn );
		}

		bool can_switch_to() const override
		{
			const Pawn* owner = machine->owner;
			if ( owner->data->max_child_spawn_count <= 0 ) return false;
			if ( owner->hunger < owner->data->min_hunger_for_reproduction ) return false;

			//	Has a group assigned and is not exceeding its population limit?
			if ( owner->group_id > 0 )
			{
				const World* world = owner->get_world();
				const int population_limit = world->get_group_limit( owner->group_id );
				if ( population_limit > 0 )
				{
					const int current_population = world->get_pawns_count_in_group( owner->group_id );
					if ( current_population >= population_limit ) return false;
				}
			}

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