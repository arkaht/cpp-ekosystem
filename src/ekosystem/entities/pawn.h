#pragma once

#include <suprengine/core/entity.h>

#include <suprengine/components/renderers/model-renderer.hpp>

#include <suprengine/utils/curve.h>

#include <ekosystem/world.h>
#include <ekosystem/components/state-machine.h>

namespace eks
{
	using namespace suprengine;

	class ParticleRenderer;

	class Pawn : public Entity
	{
	public:
		Pawn( World* world, SafePtr<PawnData> data );

		void setup() override;
		void update_this( float dt ) override;
		void tick( float dt );

		void reproduce( SafePtr<Pawn> partner );

		void set_tile_pos( const Vec3& tile_pos );
		void update_tile_pos();
		Vec3 get_tile_pos() const;

		bool can_reproduce() const;
		bool is_same_group( GroupID group_id ) const;

		const std::string& get_name() const;
		World* get_world() const;

		SafePtr<StateMachine<Pawn>> get_state_machine() const;

	public:
		SafePtr<PawnData> data = nullptr;

		GroupID group_id = 0;
		float hunger = 1.0f;

		bool wants_to_mate = false;
		bool is_sleeping = false;
		SafePtr<Pawn> partner_pawn = nullptr;

	private:
		World* _world = nullptr;
		SharedPtr<ModelRenderer> _renderer = nullptr;
		SharedPtr<StateMachine<Pawn>> _state_machine = nullptr;
		SharedPtr<ParticleRenderer> _sleep_particle_renderer = nullptr;
		SharedPtr<ParticleRenderer> _love_particle_renderer = nullptr;

		//  Position in tile coordinates
		Vec3 _tile_pos = Vec3::zero;

		std::string _name = "";
	};
}