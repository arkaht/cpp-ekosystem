#pragma once

#include <map>

#include <suprengine/entity.h>

#include <ekosystem/data/pawn-data.hpp>

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

		SafePtr<PawnData> get_pawn_data( rconst_str name ) const;

		void clear();

		Vec3 find_random_tile_pos();
		SafePtr<Pawn> find_pawn_with( 
			Adjectives adjectives, 
			SafePtr<Pawn> pawn_to_ignore 
		);

		const std::vector<SafePtr<Pawn>>& get_pawns() const;
		const std::map<std::string, SharedPtr<PawnData>>& get_pawn_datas() const;

		Vec2 get_size() const;

	public:
		const float TILE_SIZE = 10.0f;

	private:
		void _init_datas();
		void _add_pawn_data( SharedPtr<PawnData> data );

		void _on_entity_removed( Entity* entity );

	private:
		Vec2 _size;

		SafePtr<Entity> _ground;
		std::vector<SafePtr<Pawn>> _pawns;

		std::map<std::string, SharedPtr<PawnData>> _pawn_datas;
	};
}