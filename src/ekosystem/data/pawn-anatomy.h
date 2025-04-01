#pragma once

#include "pawn-stats.h"

#include <vector>

namespace eks
{
	enum class PawnBodyPartType : uint8
	{
		None,
		Head,
		Neck,
		Torso,
		Arm,
		Hand,
		Leg,
		Foot,

		Jaw,
	};

	struct PawnBodyPart
	{
		PawnBodyPartType type = PawnBodyPartType::None;
		PawnBodyPartType parent_type = PawnBodyPartType::None;

		// If set to true, the pawn will die when this body part is destroyed.
		bool is_vital = false;
		uint8 quantity = 1;

		// TODO: Support for multiple stat modifiers
		PawnStatModifier stat_modifier {};

		int32 health = 15;
		float hit_chance = 0.0f;
	};

	struct PawnBodyPartKey
	{
		PawnBodyPartType type = PawnBodyPartType::None;
		uint8 index = 0;

		bool operator==( const PawnBodyPartKey& rhs ) const
		{
			return type == rhs.type
				&& index == rhs.index;
		}
	};

	class PawnAnatomyData
	{
	public:
		PawnAnatomyData();
		PawnAnatomyData( const std::vector<PawnBodyPart>& body_parts );

		bool validate() const;

		void add_body_part( const PawnBodyPart& body_part );
		PawnBodyPart* get_body_part( PawnBodyPartType type ) const;
		const std::vector<PawnBodyPart>& get_body_parts() const;

		float get_total_body_parts_hit_chance() const;

	private:
		std::vector<PawnBodyPart> _body_parts {};
	};
};

template <>
struct std::hash<eks::PawnBodyPartKey>
{
	std::size_t operator()( const eks::PawnBodyPartKey& rhs ) const
	{
		return hash<uint8>()( static_cast<uint8>( rhs.type ) )
			 ^ ( hash<uint8>()( rhs.index ) << 1 );
	}
};