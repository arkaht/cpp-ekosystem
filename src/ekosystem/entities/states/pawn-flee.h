#pragma once

#include "tasks/pawn-flee-from.h"

namespace eks
{
	class PawnFleeState : public State<Pawn>
	{
	public:
		PawnFleeState( float radius )
			: _radius_sqr( radius * radius )
		{
			create_task<PawnFleeFromStateTask>( &_target_pawn, radius + 2.0f );
		}

		void on_begin() override
		{
			_target_pawn = _find_flee_target();
		}
		void on_end() override
		{
			_target_pawn = nullptr;
		}

		bool can_switch_to() const override
		{
			const Pawn* owner = machine->owner;
			if ( owner->data->move_speed <= 0.0f ) return false;

			const SafePtr<Pawn> target = _find_flee_target();
			if ( !target.is_valid() ) return false;

			return true;
		}

		std::string get_name() const override
		{
			return "PawnFleeState";
		}

	private:
		SafePtr<Pawn> _find_flee_target() const
		{
			const Pawn* owner = machine->owner;
			const World* world = owner->get_world();

			return world->find_pawn(
				[&]( const SafePtr<Pawn> pawn ) {
					if ( pawn.get() == owner ) return false;
					if ( pawn->data == owner->data ) return false;

					if ( owner->data->has_adjective( Adjectives::Meat ) && !pawn->data->has_adjective( Adjectives::Carnivore ) ) return false;

					const float dist_sqr = Vec3::distance2d_sqr( pawn->get_tile_pos(), owner->get_tile_pos() );
					if ( dist_sqr > _radius_sqr ) return false;

					return true;
				}
			);
		}

	private:
		SafePtr<Pawn> _target_pawn = nullptr;
		Vec3 _flee_location = Vec3::zero;

		float _radius_sqr = 0.0f;
	};
}