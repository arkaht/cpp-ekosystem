#pragma once

#include <map>

#include <suprengine/core/entity.h>

#include <suprengine/math/box.h>

#include <ekosystem/data/pawn-data.h>

namespace suprengine
{
	class ModelRenderer;
}

namespace eks
{
	using namespace suprengine;

	class Pawn;

	using GroupID = uint8_t;
	enum { MAX_PAWN_GROUP_ID = 10 };

	class World
	{
	public:
		World( const Vec2& size );
		~World();

		void update( float dt );

		SharedPtr<Pawn> create_pawn(
			SafePtr<PawnData> data,
			const Vec3& tile_pos
		);

		void add_pawn_data( SharedPtr<PawnData> data );
		SafePtr<PawnData> get_pawn_data( rconst_str name ) const;

		void set_group_limit( GroupID group_id, uint8 limit );
		int get_group_limit( GroupID group_id ) const;

		int get_pawns_count_in_group( GroupID group_id ) const;

		void resize( const Vec2& size );
		void clear();

		bool find_empty_tile_pos_around( const Vec3& pos, Vec3* out, Adjectives adjectives_filter = Adjectives::None ) const;
		Vec3 find_random_tile_pos() const;
		SafePtr<Pawn> find_pawn_with(
			Adjectives adjectives,
			SafePtr<Pawn> pawn_to_ignore
		) const;
		SafePtr<Pawn> find_pawn_at(
			Adjectives adjectives,
			const Vec3& pos
		) const;
		SafePtr<Pawn> find_nearest_pawn(
			const Vec3& origin,
			std::function<bool( SafePtr<Pawn> )> callback
		) const;
		SafePtr<Pawn> find_pawn(
			std::function<bool( SafePtr<Pawn> )> callback
		) const;

		Vec3 world_to_grid( const Vec3& world_pos ) const;
		Vec3 grid_to_world( const Vec3& grid_pos ) const;

		const std::vector<SafePtr<Pawn>>& get_pawns() const;
		std::map<std::string, SharedPtr<PawnData>>& get_pawn_datas();
		const std::map<std::string, SharedPtr<PawnData>>& get_pawn_datas() const;
		
		Vec2 get_size() const;
		Box get_bounds() const;

		Vec3 get_sun_direction() const;
		float get_photosynthesis_multiplier() const;

		bool is_within_world_time( float min_hours, float max_hours ) const;
		float get_world_time() const;

	public:
		const float TILE_SIZE = 10.0f;

		float world_time_scale = 0.5f;

	private:
		void _init_datas();

		void _on_entity_removed( Entity* entity );

	private:
		float _world_time = 0.0f;
		Vec3 _sun_direction = Vec3::zero;
		float _photosynthesis_multiplier = 0.0f;

		Vec2 _size = Vec2::zero;

		SafePtr<Entity> _ground = nullptr;
		SafePtr<Entity> _sun = nullptr;
		SafePtr<Entity> _moon = nullptr;
		SafePtr<ModelRenderer> _ground_renderer = nullptr;
		std::vector<SafePtr<Pawn>> _pawns {};

		std::map<std::string, SharedPtr<PawnData>> _pawn_datas {};

		uint8 _group_limits[MAX_PAWN_GROUP_ID + 1] {};
	};
}