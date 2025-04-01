#pragma once

#include <suprengine/core/entity.h>

#include <suprengine/components/renderers/model-renderer.hpp>

#include <suprengine/utils/curve.h>

#include <ekosystem/world.h>
#include <ekosystem/components/state-machine.h>
#include <ekosystem/data/pawn-anatomy.h>
#include <ekosystem/data/pawn-health.h>

namespace eks
{
	using namespace suprengine;

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
		PawnStatManager stats_manager;
		PawnHealthSystem health_system;

		GroupID group_id = 0;
		float hunger = 1.0f;

		bool wants_to_mate = false;

	private:
		World* _world = nullptr;
		SharedPtr<ModelRenderer> _renderer = nullptr;
		SharedPtr<StateMachine<Pawn>> _state_machine = nullptr;

		//  Position in tile coordinates
		Vec3 _tile_pos = Vec3::zero;

		std::string _name = "";
	};
}