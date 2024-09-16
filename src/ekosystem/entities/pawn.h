#pragma once

#include <suprengine/entity.h>
#include <suprengine/components/renderers/model-renderer.hpp>

#include <ekosystem/world.h>

namespace eks
{
	using namespace suprengine;

	using GroupID = uint8_t;

	class Pawn : public Entity
	{
	public:
		Pawn( World* world, SafePtr<PawnData> data );

		void setup() override;
		void update_this( float dt ) override;
		void tick( float dt );

		void move_to( const Vec3& target );
		void reproduce();

		void set_tile_pos( const Vec3& tile_pos );
		void update_tile_pos();
		Vec3 get_tile_pos() const;

		bool can_reproduce() const;
		bool is_same_group( GroupID group_id ) const;

		std::string get_name() const;

	public:
		SafePtr<PawnData> data = nullptr;

		GroupID group_id = 0;
		float hunger = 1.0f;

	private:
		void _find_food();
		void _find_partner();
		void _find_path_to( const Vec3& target );

	private:
		World* _world = nullptr;
		SharedPtr<ModelRenderer> _renderer = nullptr;

		SafePtr<Pawn> _food_target = nullptr;
		SafePtr<Pawn> _partner_target = nullptr;

		std::vector<Vec3> _move_path;

		float _move_progress = 1.0f;
		Vec3 _move_to = Vec3::zero;

		//  Position in tile coordinates
		Vec3 _tile_pos = Vec3::zero;
	};
}