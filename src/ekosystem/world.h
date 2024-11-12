#pragma once

#include <map>

#include <suprengine/entity.h>
#include <suprengine/box.h>

#include <ekosystem/data/pawn-data.h>

namespace eks
{
	using namespace suprengine;

	class Pawn;

	class World
	{
	public:
		World( const Vec2& size );
		~World();

		SharedPtr<Pawn> create_pawn(
			SafePtr<PawnData> data,
			const Vec3& tile_pos
		);

		void add_pawn_data( SharedPtr<PawnData> data );
		SafePtr<PawnData> get_pawn_data( rconst_str name ) const;

		void resize( const Vec2& size );
		void clear();

		bool find_empty_tile_pos_around( const Vec3& pos, Vec3* out ) const;
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

		Vec3 world_to_grid( const Vec3& world_pos );

		const std::vector<SafePtr<Pawn>>& get_pawns() const;
		std::map<std::string, SharedPtr<PawnData>>& get_pawn_datas();
		const std::map<std::string, SharedPtr<PawnData>>& get_pawn_datas() const;
		
		Vec2 get_size() const;
		Box get_bounds() const;

	public:
		const float TILE_SIZE = 10.0f;

	private:
		void _init_datas();

		void _on_entity_removed( Entity* entity );

	private:
		Vec2 _size = Vec2::zero;

		SafePtr<Entity> _ground = nullptr;
		std::vector<SafePtr<Pawn>> _pawns {};

		std::map<std::string, SharedPtr<PawnData>> _pawn_datas {};
	};
}