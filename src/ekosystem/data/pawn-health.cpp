#include "pawn-health.h"
#include "pawn-health.h"

#include <suprengine/utils/random.h>

#include <ekosystem/entities/pawn.h>

#include <string>

using namespace eks;

PawnHealthSystem::PawnHealthSystem( Pawn& pawn )
	: _pawn( pawn )
{}

void PawnHealthSystem::initialize()
{
	_initialize_body_parts();
}

bool PawnHealthSystem::take_damage( const DamageContext& context )
{
	const int32 pre_damage_health = get_body_part_health( context.hit_body_part );
	if ( pre_damage_health <= 0 ) return false;

	const int32 post_damage_health = _body_parts_health[context.hit_body_part] -= context.damage;
	if ( post_damage_health <= 0 )
	{
		printf(
			"%s lost body part %d:%d\n",
			*_pawn.get_name(),
			context.hit_body_part.type, context.hit_body_part.index
		);
	}

	printf(
		"%s took %d damage in body part %d:%d and now has %d HP\n",
		*_pawn.get_name(),
		context.damage,
		context.hit_body_part.type, context.hit_body_part.index,
		post_damage_health
	);
	return true;
}

int32 PawnHealthSystem::get_body_part_health( PawnBodyPartType body_part_type, uint8 body_part_index ) const
{
	PawnBodyPartKey key {
		.type = body_part_type,
		.index = body_part_index,
	};

	return get_body_part_health( key );
}

int32 PawnHealthSystem::get_body_part_health( PawnBodyPartKey body_part_key ) const
{
	auto itr = _body_parts_health.find( body_part_key );
	if ( itr == _body_parts_health.end() ) return 0;

	return itr->second;
}

void PawnHealthSystem::get_random_body_part_by_hit_chance( PawnBodyPartKey& out_body_part_key ) const
{
	const float chance = random::generate( 0.0f, 1.0f );

	float accumulated_hit_chance = 0.0f;
	for ( const PawnBodyPart& body_part : _pawn.data->anatomy->get_body_parts() )
	{
		accumulated_hit_chance += body_part.hit_chance;

		if ( accumulated_hit_chance > chance )
		{
			if ( body_part.quantity > 1 )
			{
				std::vector<uint8> available_indices {};
				for ( int i = 0; i < body_part.quantity; i++ )
				{
					if ( get_body_part_health( body_part.type, i ) <= 0 ) continue;
					available_indices.push_back( i );
				}

				// If we really find no alive body part, skip that
				if ( available_indices.empty() ) continue;

				out_body_part_key.type = body_part.type;
				out_body_part_key.index = random::generate( available_indices );
			}
			else
			{
				out_body_part_key.type = body_part.type;
				out_body_part_key.index = 0;
			}
			return;
		}
	}
}

void PawnHealthSystem::_initialize_body_parts()
{
	printf( "%s", *_pawn.get_name() );
	for ( const PawnBodyPart& body_part : _pawn.data->anatomy->get_body_parts() )
	{
		for ( uint8 i = 0; i < body_part.quantity; i++ )
		{
			PawnBodyPartKey key {
				.type = body_part.type,
				.index = i,
			};

			_body_parts_health[key] = body_part.health;
			printf( "BodyPart %d:%d = %dHP\n", key.type, key.index, body_part.health );
		}
	}
}
