#pragma once

#include "pawn-anatomy.h"

#include <unordered_map>

namespace eks
{
	class Pawn;

	struct DamageContext
	{
		int32 damage = 0;

		PawnBodyPartKey hit_body_part {};
	};

	class PawnHealthSystem
	{
	public:
		PawnHealthSystem( Pawn& pawn );

		void initialize();

		bool take_damage( const DamageContext& context );

		int32 get_body_part_health( PawnBodyPartType body_part_type, uint8 body_part_index = 0 ) const;
		int32 get_body_part_health( PawnBodyPartKey body_part_key ) const;
		void get_random_body_part_by_hit_chance( PawnBodyPartKey& out_body_part_key ) const;

	private:
		void _initialize_body_parts();

	private:
		Pawn& _pawn;

		std::unordered_map<PawnBodyPartKey, int32> _body_parts_health {};
	};
}