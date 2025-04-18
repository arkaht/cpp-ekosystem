#include "pawn-data.h"

using namespace suprengine;
using namespace eks;

#define JSON_KEY( name ) #name, name
#define JSON_KEY_REF( name ) #name, &name

bool PawnData::serialize( json::document& doc ) const
{
	doc.SetObject();

	json::add( doc, JSON_KEY( model_name ) );
	json::add( doc, JSON_KEY( shader_name ) );
	json::add( doc, JSON_KEY( modulate ) );

	json::add( doc, JSON_KEY( movement_progress_curve_name ) );
	json::add( doc, JSON_KEY( movement_height_curve_name ) );
	json::add( doc, JSON_KEY( movement_scale_y_curve_name ) );

	json::add( doc, JSON_KEY( move_speed ) );
	json::add( doc, JSON_KEY( start_sleep_time ) );
	json::add( doc, JSON_KEY( end_sleep_time ) );

	json::add( doc, JSON_KEY( min_child_spawn_count ) );
	json::add( doc, JSON_KEY( max_child_spawn_count ) );
	json::add( doc, JSON_KEY( min_hunger_for_reproduction ) );
	json::add( doc, JSON_KEY( hunger_consumption_on_reproduction ) );

	json::add( doc, JSON_KEY( food_amount ) );
	json::add( doc, JSON_KEY( max_hunger ) );
	json::add( doc, JSON_KEY( natural_hunger_consumption ) );
	json::add( doc, JSON_KEY( min_hunger_to_eat ) );
	json::add( doc, JSON_KEY( hunger_at_spawn ) );

	json::add( doc, JSON_KEY( photosynthesis_gain ) );

	json::add( doc, "adjectives", static_cast<uint32_t>( adjectives ) );

	return true;
}

bool PawnData::unserialize( const json::document& doc )
{
	json::get( doc, JSON_KEY_REF( model_name ), std::string( MESH_CUBE ) );
	json::get( doc, JSON_KEY_REF( shader_name ), std::string( SHADER_LIT_MESH ) );
	json::get( doc, JSON_KEY_REF( modulate ), Color::white );

	json::get( doc, JSON_KEY_REF( movement_progress_curve_name ) );
	json::get( doc, JSON_KEY_REF( movement_height_curve_name ) );
	json::get( doc, JSON_KEY_REF( movement_scale_y_curve_name ) );

	json::get( doc, JSON_KEY_REF( move_speed ) );
	json::get( doc, JSON_KEY_REF( start_sleep_time ) );
	json::get( doc, JSON_KEY_REF( end_sleep_time ) );

	json::get( doc, JSON_KEY_REF( max_child_spawn_count ) );
	json::get( doc, JSON_KEY_REF( min_child_spawn_count ), max_child_spawn_count );
	json::get( doc, JSON_KEY_REF( min_hunger_for_reproduction ) );
	json::get( doc, JSON_KEY_REF( hunger_consumption_on_reproduction ) );

	json::get( doc, JSON_KEY_REF( food_amount ) );
	json::get( doc, JSON_KEY_REF( max_hunger ) );
	json::get( doc, JSON_KEY_REF( natural_hunger_consumption ) );
	json::get( doc, JSON_KEY_REF( min_hunger_to_eat ) );
	json::get( doc, JSON_KEY_REF( hunger_at_spawn ), hunger_at_spawn );

	json::get( doc, JSON_KEY_REF( photosynthesis_gain ) );

	adjectives = static_cast<Adjectives>( json::get( doc, "adjectives", 0Ui32 ) );

	return true;
}
