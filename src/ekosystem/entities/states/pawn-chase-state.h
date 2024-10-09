#pragma once

#include <ekosystem/components/state-machine.h>

#include "pawn.h"

namespace eks
{
	class PawnFindPawnStateTask : public StateTask<Pawn>
	{
	public:
		PawnFindPawnStateTask( SafePtr<Pawn>* target_key )
			: _target_key( target_key )
		{};

		void on_update( float dt ) override
		{
			printf( "PawnFindPawnStateTask update\n" );

			if ( find_food() )
			{
				state->next_task();
			}
		}

		bool find_food()
		{
			Pawn* owner = state->machine->owner;
			const World* world = owner->get_world();

			if ( owner->data->has_adjective( Adjectives::Herbivore ) )
			{
				auto target = world->find_nearest_pawn(
					owner->get_tile_pos(),
					[&]( auto pawn )
					{
						if ( pawn.get() == owner ) return false;
						if ( pawn->is_same_group( owner->group_id ) ) return false;
						return pawn->data->has_adjective( Adjectives::Vegetal );
					}
				);
				if ( !target.is_valid() )
				{
					printf(
						"'%s' can't find any vegetal to eat!\n",
						owner->get_name().c_str()
					);
					return;
				}

				*_target_key = target;
				printf(
					"Herbivore '%s' wants to eat '%s'!\n",
					owner->get_name().c_str(), target->get_name().c_str()
				);
				return true;
			}
			else if ( owner->data->has_adjective( Adjectives::Carnivore ) )
			{
				auto target = world->find_nearest_pawn(
					owner->get_tile_pos(),
					[&]( auto pawn )
					{
						if ( pawn.get() == owner ) return false;
						if ( pawn->is_same_group( owner->group_id ) ) return false;
						return pawn->data->has_adjective( Adjectives::Meat );
					}
				);
				if ( !target.is_valid() )
				{
					printf(
						"'%s' can't find any meat to eat!\n",
						owner->get_name().c_str()
					);
					return;
				}

				*_target_key = target;
				printf(
					"Carnivore '%s' wants to eat '%s'!\n",
					owner->get_name().c_str(), target->get_name().c_str()
				);
				return true;
			}
			else if ( owner->data->has_adjective( Adjectives::Photosynthesis ) )
			{
				owner->hunger = owner->data->max_hunger;
				printf(
					"Photosynthesis '%s' wants to eat!\n",
					owner->get_name().c_str()
				);
				return true;
			}
		}

	private:
		SafePtr<Pawn>* _target_key = nullptr;
	};

	class PawnMoveStateTask : public StateTask<Pawn>
	{
	public:
		PawnMoveStateTask() {};

		void on_begin() override
		{
			_timer = 0.0f;
		}
		void on_update( float dt ) override
		{
			printf( "PawnMoveStateTask update\n" );

			_timer += dt;
			if ( _timer > 2.0f )
			{
				state->next_task();
			}
		}

	private:
		float _timer = 0.0f;
	};

	class PawnEatStateTask : public StateTask<Pawn>
	{
	public:
		PawnEatStateTask() {};

		void on_update( float dt ) override
		{
			printf( "PawnEatStateTask update\n" );
			state->next_task();
		}
	};

	class PawnChaseState : public State<Pawn>
	{
	public:
		PawnChaseState()
		{
			create_task<PawnFindPawnStateTask>( &_target );
			create_task<PawnMoveStateTask>();
			create_task<PawnEatStateTask>();
		}

		void on_update( float dt ) override
		{
			printf( "PawnChaseState update\n" );
		}

	private:
		SafePtr<Pawn> _target = nullptr;
	};
}