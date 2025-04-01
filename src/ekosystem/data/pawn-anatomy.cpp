#include "pawn-anatomy.h"

#include <suprengine/math/math.h>
#include <suprengine/utils/logger.h>

using namespace eks;
using namespace suprengine;

PawnAnatomyData::PawnAnatomyData() {}

PawnAnatomyData::PawnAnatomyData( const std::vector<PawnBodyPart>& body_parts )
	: _body_parts( body_parts )
{
	validate();
}

bool PawnAnatomyData::validate() const
{
	float total_hit_chance = 0.0f;
	int32 total_health = 0;

	for ( const PawnBodyPart& body_part : _body_parts )
	{
		total_health += body_part.health;
		total_hit_chance += body_part.hit_chance;
	}

	if ( !math::near_value( total_hit_chance, 1.0f ) )
	{
		Logger::error(
			"Failed to validate PawnAnatomyData: total hit chance is equal to %.01f instead of 100.0",
			total_hit_chance * 100.0f
		);
		return false;
	}

	Logger::info(
		"Validated a PawnAnatomyData (HP: %d; HC: %.01f)",
		total_health, total_hit_chance
	);
	return true;
}

void PawnAnatomyData::add_body_part( const PawnBodyPart& body_part )
{
	_body_parts.push_back( body_part );
}

PawnBodyPart* PawnAnatomyData::get_body_part( PawnBodyPartType type ) const
{
	for ( const PawnBodyPart& body_part : _body_parts )
	{
		if ( body_part.type == type )
		{
			return const_cast<PawnBodyPart*>( &body_part );
		}
	}

	return nullptr;
}

const std::vector<PawnBodyPart>& eks::PawnAnatomyData::get_body_parts() const
{
	return _body_parts;
}

float PawnAnatomyData::get_total_body_parts_hit_chance() const
{
	float total_hit_chance = 0.0f;
	for ( const PawnBodyPart& body_part : _body_parts )
	{
		total_hit_chance += body_part.hit_chance;
	}

	return total_hit_chance;
}
