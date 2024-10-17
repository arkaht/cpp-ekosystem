#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

namespace eks
{
	class PawnFindMateStateTask : public StateTask<Pawn>
	{
	public:
		PawnFindMateStateTask( SafePtr<Pawn>* target_key )
			: target_key( target_key )
		{};

		void on_begin() override
		{
			Pawn* owner = state->machine->owner;
			owner->wants_to_mate = true;

			if ( owner->data->move_speed <= 0.0f )
			{
				finish( StateTaskResult::Failed );
			}
		}
		void on_update( float dt ) override
		{
			if ( !find_mate() ) return;

			finish( StateTaskResult::Succeed );
		}
		void on_end() override
		{
			Pawn* owner = state->machine->owner;
			owner->wants_to_mate = false;
		}

		bool can_ignore() const override
		{
			const Pawn* owner = state->machine->owner;
			if ( owner->data->has_adjective( Adjectives::Photosynthesis ) ) return true;

			return false;
		}

		bool find_mate()
		{
			Pawn* owner = state->machine->owner;
			const World* world = owner->get_world();

			auto mate_pawn = world->find_nearest_pawn(
				owner->get_tile_pos(),
				[&]( auto pawn )
				{
					if ( pawn.get() == owner ) return false;
					return pawn->data == owner->data && pawn->wants_to_mate;
				}
			);
			if ( !mate_pawn.is_valid() ) return false;

			printf(
				"'%s' will mate with '%s'\n",
				*owner->get_name(), *mate_pawn->get_name()
			);

			*target_key = mate_pawn;

			return true;
		}

	public:
		SafePtr<Pawn>* target_key = nullptr;
	};
}