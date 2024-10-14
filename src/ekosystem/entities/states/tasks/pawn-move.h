#pragma once

#include <ekosystem/components/state-machine.h>

#include "pawn.h"

namespace eks
{
	class PawnMoveStateTask : public StateTask<Pawn>
	{
	public:
		PawnMoveStateTask( SafePtr<Pawn>* target_key )
			: pawn_target_key( target_key )
		{};
		PawnMoveStateTask( Vec3* target_key )
			: location_target_key( target_key )
		{};

		void on_begin() override
		{
			_move_progress = 0.0f;
		}
		void on_update( float dt ) override
		{
			Pawn* owner = state->machine->owner;
			const World* world = owner->get_world();

			//	Update path only when not moving
			if ( !is_moving() && !_update_path() )
			{
				finish( StateTaskResult::Failed );
				return;
			}

			if ( _move_path.size() == 0 )
			{
				finish( StateTaskResult::Succeed );
				return;
			}

			//  Increase movement progress
			_move_progress = math::min( 
				_move_progress + owner->data->move_speed * dt, 
				1.0f 
			);

			//  Compute this frame position
			const Vec3 next_tile = _move_path.at( 0 );
			Vec3 new_tile_pos = Vec3::lerp( 
				owner->get_tile_pos(),
				next_tile,
				_move_progress
			);

			//	Complete this movement step
			if ( _move_progress >= 1.0f )
			{
				owner->set_tile_pos( new_tile_pos );

				_move_progress = 0.0f;
				//  TODO: Refactor to erase from end
				_move_path.erase( _move_path.begin() );
			}

			//	Apply render position
			Vec3 render_pos = new_tile_pos * world->TILE_SIZE;
			if ( owner->movement_height_curve )
			{
				render_pos.z = owner->movement_height_curve->evaluate_by_time( _move_progress );
			}
			owner->transform->set_location( render_pos );
		}

		bool can_switch_from() const override
		{
			//	Delay switching state until the movement has ended
			return !is_moving();
		}

		bool is_moving() const
		{
			return _move_progress > 0.0f;
		}

	public:
		SafePtr<Pawn>* pawn_target_key = nullptr;
		Vec3* location_target_key = nullptr;

	private:
		bool _update_path()
		{
			if ( pawn_target_key != nullptr && pawn_target_key->is_valid() )
			{
				const Vec3 tile_pos = ( *pawn_target_key )->get_tile_pos();

				//	Update path only if different
				if ( tile_pos != _target_pos )
				{
					_find_path_to( tile_pos );
				}

				return true;
			}
			else if ( location_target_key != nullptr )
			{
				_find_path_to( *location_target_key );
				return true;
			}

			//	No valid key was found to find a path; aborting.
			return false;
		}
		void _find_path_to( const Vec3& target )
		{
			const Pawn* owner = state->machine->owner;

			_move_path.clear();

			const Vec3 location = owner->get_tile_pos();
			const Vec3 diff = target - location;

			const float x_sign = math::sign( diff.x );
			const int max_off_x = static_cast<int>( math::abs( diff.x ) ) + 1;
			for ( int off_x = 1; off_x < max_off_x; off_x++ )
			{
				_move_path.push_back( 
					Vec3 {
						location.x + off_x * x_sign,
						location.y,
						location.z
					}
				);
			}

			const float y_sign = math::sign( diff.y );
			const int max_off_y = static_cast<int>( math::abs( diff.y ) ) + 1;
			for ( int off_y = 1; off_y < max_off_y; off_y++ )
			{
				_move_path.push_back(
					Vec3 {
						target.x,
						location.y + off_y * y_sign,
						location.z
					}
				);
			}
		}

	private:
		std::vector<Vec3> _move_path {};

		bool _is_moving = false;
		float _move_progress = 0.0f;
		Vec3 _target_pos = Vec3::zero;
	};
}