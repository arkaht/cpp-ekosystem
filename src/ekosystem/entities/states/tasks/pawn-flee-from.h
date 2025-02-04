#pragma once

#include "pawn-move.h"

namespace eks
{
	class PawnFleeFromStateTask : public PawnMoveStateTask
	{
	public:
		PawnFleeFromStateTask( SafePtr<Pawn>* target_key, float radius )
			: _flee_target_key( target_key ), _radius_sqr( radius * radius ),
			  PawnMoveStateTask( &_flee_location )
		{}

		void on_begin() override
		{
			PawnMoveStateTask::on_begin();

			_last_target_location = Vec3::zero;

			if ( _flee_target_key->is_valid() )
			{
				_update_flee_location();
			}
		}
		void on_update( float dt ) override
		{
			if ( !_flee_target_key->is_valid() )
			{
				if ( !is_moving() )
				{
					finish( StateTaskResult::Succeed );
					return;
				}

				PawnMoveStateTask::on_update( dt );
				return;
			}

			if ( _last_target_location != ( *_flee_target_key )->get_tile_pos() )
			{
				_update_flee_location();
			}

			//	Visual debug
			if ( VisDebug::is_channel_active( DebugChannel::AI ) )
			{
				const Pawn* owner = state->machine->owner;
				const World* world = owner->get_world();

				const Vec3 owner_world_location = world->grid_to_world( owner->get_tile_pos() );
				const Vec3 flee_world_location = world->grid_to_world( _flee_location );
				const Vec3 target_world_location = world->grid_to_world( _last_target_location );

				VisDebug::add_sphere( flee_world_location, 1.0f, Color::purple, 0.0f, DebugChannel::AI );
				VisDebug::add_line( owner_world_location, flee_world_location, Color::purple, 0.0f, DebugChannel::AI );

				VisDebug::add_sphere( target_world_location, 0.5f, Color::duckblue, 0.0f, DebugChannel::AI );
				VisDebug::add_line( owner_world_location, target_world_location, Color::duckblue, 0.0f, DebugChannel::AI );
			}

			PawnMoveStateTask::on_update( dt );
		}

		bool can_switch_from() const override
		{
			const bool can_switch = PawnMoveStateTask::can_switch_from();
			if ( !can_switch ) return false;

			//	Check the pawn is out-of-range from the target
			if ( _flee_target_key->is_valid() )
			{
				const Pawn* owner = state->machine->owner;
				const float dist_sqr = Vec3::distance2d_sqr( owner->get_tile_pos(), ( *_flee_target_key )->get_tile_pos() );
				return dist_sqr >= _radius_sqr;
			}

			return true;
		}

		std::string get_name() const override
		{
			return "PawnFleeFromStateTask";
		}

	private:
		void _update_flee_location()
		{
			const Pawn* owner = state->machine->owner;
			const Vec3 owner_location = owner->get_tile_pos();
			const World* world = owner->get_world();
			const Box bounds = world->get_bounds();

			_last_target_location = ( *_flee_target_key )->get_tile_pos();

			//	Compute flee direction
			Vec3 flee_direction = Vec3::direction2d( _last_target_location, owner_location );

			//	Flee perpendicular from the original flee direction when near world edges
			bool is_on_world_edge = owner_location.x == bounds.min.x || owner_location.x == bounds.max.x
								 || owner_location.y == bounds.min.y || owner_location.y == bounds.max.y;
			if ( is_on_world_edge )
			{
				flee_direction = Vec3::cross( flee_direction, Vec3::up );
			}

		#ifdef ENABLE_VISDEBUG
			//	Draw flee location
			VisDebug::add_line(
				world->grid_to_world( owner_location ),
				world->grid_to_world( owner_location ) + world->grid_to_world( flee_direction ),
				Color::purple, 1.0f,
				DebugChannel::AI
			);
		#endif

			//	Round to a valid tile location
			_flee_location = Vec3::round( owner_location + flee_direction );

		#ifdef ENABLE_VISDEBUG
			//	Draw un-clamped flee location
			VisDebug::add_box( world->grid_to_world( _flee_location ), Quaternion::identity, Box::half, Color::purple, 1.0f, DebugChannel::AI );
		#endif

			_flee_location = Vec3::clamp( _flee_location, bounds.min, bounds.max );
		}

	private:
		SafePtr<Pawn>* _flee_target_key = nullptr;

		Vec3 _flee_location = Vec3::zero;
		Vec3 _last_target_location = Vec3::zero;

		float _radius_sqr = 0.0f;
	};
}