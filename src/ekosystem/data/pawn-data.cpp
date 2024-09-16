#include "pawn-data.h"

using namespace suprengine;
using namespace eks;

bool PawnData::serialize( json::document& doc ) const
{
	doc.SetObject();

	json::add( doc, "model_name", model_name );
	json::add( doc, "modulate", modulate );

	json::add( doc, "move_speed", move_speed );

	json::add( doc, "min_child_spawn_count", min_child_spawn_count );
	json::add( doc, "max_child_spawn_count", max_child_spawn_count );
	json::add( doc, "min_hunger_for_reproduction", min_hunger_for_reproduction );
	json::add( doc, "hunger_consumption_on_reproduction", hunger_consumption_on_reproduction );

	json::add( doc, "food_amount", food_amount );
	json::add( doc, "max_hunger", max_hunger );
	json::add( doc, "natural_hunger_consumption", natural_hunger_consumption );
	json::add( doc, "min_hunger_to_eat", min_hunger_to_eat );

	json::add( doc, "photosynthesis_gain", photosynthesis_gain );

	json::add( doc, "adjectives", (uint32_t)adjectives );

	return true;
}

bool PawnData::unserialize( const json::document& doc )
{
	json::get( doc, "model_name", &model_name, MESH_CUBE );
	json::get( doc, "modulate", &modulate, Color::white );

	json::get( doc, "move_speed", &move_speed );

	json::get( doc, "max_child_spawn_count", &max_child_spawn_count );
	json::get( doc, "min_child_spawn_count", &min_child_spawn_count, max_child_spawn_count );
	json::get( doc, "min_hunger_for_reproduction", &min_hunger_for_reproduction );
	json::get( doc, "hunger_consumption_on_reproduction", &hunger_consumption_on_reproduction );

	json::get( doc, "food_amount", &food_amount );
	json::get( doc, "max_hunger", &max_hunger );
	json::get( doc, "natural_hunger_consumption", &natural_hunger_consumption );
	json::get( doc, "min_hunger_to_eat", &min_hunger_to_eat );

	json::get( doc, "photosynthesis_gain", &photosynthesis_gain );

	adjectives = (Adjectives)json::get( doc, "adjectives", 0Ui32 );

	return true;
}
