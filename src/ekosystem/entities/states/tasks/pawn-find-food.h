#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

namespace eks
{
	class PawnFindFoodStateTask : public StateTask<Pawn>
	{
	public:
		PawnFindFoodStateTask( SafePtr<Pawn>* target_key )
			: target_key( target_key )
		{};

		void on_begin() override
		{
			Pawn* owner = state->machine->owner;

			if ( owner->data->has_adjective( Adjectives::Photosynthesis ) )
			{
				finish( StateTaskResult::Failed );
			}
		}
		void on_update( float dt ) override
		{
			if ( !find_food() ) return;

			finish( StateTaskResult::Succeed );
		}

		std::string get_name() const override
		{
			return "PawnFindFoodStateTask";
		}

		bool find_food()
		{
			Pawn* owner = state->machine->owner;
			const World* world = owner->get_world();

			if ( owner->data->has_adjective( Adjectives::Herbivore ) )
			{
				//  Make herbivore pawns find vegetal pawns as a meal

				auto target = world->find_nearest_pawn(
					owner->get_tile_pos(),
					[&]( auto pawn )
					{
						if ( pawn.get() == owner ) return false;
						if ( pawn->is_same_group( owner->group_id ) ) return false;
						return pawn->data->has_adjective( Adjectives::Vegetal );
					}
				);
				if ( !target.is_valid() ) return false;

				*target_key = target;
				return true;
			}
			else if ( owner->data->has_adjective( Adjectives::Carnivore ) )
			{
				//  Make carnivore pawns find meat pawns as a meal

				auto target = world->find_nearest_pawn(
					owner->get_tile_pos(),
					[&]( auto pawn )
					{
						if ( pawn.get() == owner ) return false;
						if ( pawn->is_same_group( owner->group_id ) ) return false;
						return pawn->data->has_adjective( Adjectives::Meat );
					}
				);
				if ( !target.is_valid() ) return false;

				*target_key = target;
				return true;
			}

			return false;
		}

	public:
		SafePtr<Pawn>* target_key = nullptr;
	};
}