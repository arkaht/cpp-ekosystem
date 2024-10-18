#pragma once

#include <ekosystem/components/state-machine.h>

#include <ekosystem/entities/pawn.h>

namespace eks
{
	class PawnFindWanderStateTask : public StateTask<Pawn>
	{
	public:
		PawnFindWanderStateTask( Vec3* location_key )
			: location_key( location_key )
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
			const Vec3 spread = random::generate_location(
				-radius, -radius, 0.0f,
				 radius,  radius, 0.0f
			);

			Pawn* owner = state->machine->owner;
			*location_key = Vec3::clamp(
				Vec3::round( owner->get_tile_pos() + spread ),
				Vec3::zero,
				Vec3( owner->get_world()->get_size() )
			);

			finish( StateTaskResult::Succeed );
		}

		std::string get_name() const override
		{
			return "PawnFindWanderStateTask";
		}

	public:
		float radius = 2.0f;
		Vec3* location_key = nullptr;
	};
}