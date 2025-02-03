#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

#include <suprengine/tools/vis-debug.h>

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
			const Vec3 current_tile = owner->get_tile_pos();
			const Vec3 next_tile = _move_path.at( 0 );
			const Vec3 new_tile_pos = Vec3::lerp( 
				current_tile,
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
			else
			{
				//	Update render rotation
				const Quaternion target_rotation = Quaternion::look_at( current_tile, next_tile, Vec3::up );
				owner->transform->set_rotation( 
					Quaternion::slerp(
						owner->transform->rotation,
						target_rotation,
						dt * 15.0f
					)
				);
			}

			//	Apply render position
			Vec3 render_pos = new_tile_pos * world->TILE_SIZE;
			if ( owner->movement_height_curve )
			{
				render_pos.z = owner->movement_height_curve->evaluate_by_time( _move_progress );
			}
			if ( owner->movement_scale_y_curve )
			{
				Vec3 render_scale = Vec3::one;
				render_scale.z = owner->movement_scale_y_curve->evaluate_by_time( _move_progress );

				owner->transform->set_scale( render_scale );
			}
			owner->transform->set_location( render_pos );

			//	Visual debug for pathfinding
			if ( VisDebug::is_channel_active( DebugChannel::Pathfinding ) )
			{
				Vec3 last_world_pos = owner->transform->location;
				for ( int i = 0; i < _move_path.size(); i++ )
				{
					const Vec3& point = _move_path[i];
					const Vec3& world_pos = world->grid_to_world( point );

					VisDebug::add_box(
						world_pos,
						Quaternion::identity,
						i == _move_path.size() - 1 ? Box::half : Box::half * 0.5f,
						Color::red,
						0.0f,
						DebugChannel::Pathfinding
					);
					VisDebug::add_line(
						last_world_pos,
						world_pos,
						Color::red,
						0.0f,
						DebugChannel::Pathfinding
					);

					last_world_pos = world_pos;
				}
			}
		}
		void on_end() override
		{
			Pawn* owner = state->machine->owner;
			owner->transform->set_scale( Vec3::one );
		}

		bool can_switch_from() const override
		{
			//	Delay switching state until the movement has ended
			return !is_moving();
		}

		bool can_ignore() const override
		{
			const Pawn* owner = state->machine->owner;
			if ( owner->data->move_speed <= 0.0f ) return true;

			return false;
		}

		std::string get_name() const override
		{
			return "PawnMoveStateTask";
		}

		bool is_moving() const
		{
			return _move_progress > 0.0f;
		}

	public:
		SafePtr<Pawn>* pawn_target_key = nullptr;
		Vec3* location_target_key = nullptr;

	private:
		/*
		 * Try to update the path using the appropriate target key as the goal.
		 * Returns whenever the path to the goal is valid.
		 */
		bool _update_path()
		{
			Vec3 target_pos = Vec3::zero;
			if ( pawn_target_key != nullptr )
			{
				if ( !pawn_target_key->is_valid() ) return false;

				target_pos = ( *pawn_target_key )->get_tile_pos();
			}
			else if ( location_target_key != nullptr )
			{
				target_pos = *location_target_key;
			}
			else
			{
				//	No valid key was found to find a path; aborting.
				return false;
			}

			//	Check that new position is different
			if ( target_pos == _target_pos ) return true;
			
			_find_path_to( target_pos );
			return true;
		}
		void _find_path_to( const Vec3& target )
		{
			//	NOTE: Since we don't have obstacles yet, a simple algorithm
			//	is enough for pathfinding. We're just adding all points in
			//	line with both axes.

			_move_path.clear();

			const Pawn* owner = state->machine->owner;
			const Vec3 location = owner->get_tile_pos();
			const Vec3 diff = target - location;

			//	Moving on X-axis
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

			//	Moving on Y-axis
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

			_target_pos = target;
		}

	private:
		std::vector<Vec3> _move_path {};

		bool _is_moving = false;
		float _move_progress = 0.0f;
		Vec3 _target_pos = Vec3::zero;
	};
}