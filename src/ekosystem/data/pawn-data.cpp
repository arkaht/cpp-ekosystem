#include "pawn-data.h"

using namespace suprengine;
using namespace eks;

bool PawnData::serialize( json::document& doc ) const
{
	doc.SetObject();

	json::add( doc, "model_name", model_name );
	json::add( doc, "modulate", modulate );

	json::add( doc, "move_speed", move_speed );

	json::add( doc, "child_spawn_count", child_spawn_count );
	json::add( doc, "min_food_reproduction", min_food_reproduction );
	json::add( doc, "food_loss_on_reproduction", food_loss_on_reproduction );

	json::add( doc, "food_amount", food_amount );
	json::add( doc, "max_hunger", max_hunger );
	json::add( doc, "hunger_gain_rate", hunger_gain_rate );
	json::add( doc, "min_hunger_to_eat", min_hunger_to_eat );

	json::add( doc, "adjectives", (uint32_t)adjectives );

	return true;
}

bool PawnData::unserialize( const json::document& doc )
{
	json::get( doc, "model_name", &model_name, MESH_CUBE );
	json::get( doc, "modulate", &modulate, Color::white );

	json::get( doc, "move_speed", &move_speed );

	json::get( doc, "child_spawn_count", &child_spawn_count );
	json::get( doc, "min_food_reproduction", &min_food_reproduction );
	json::get( doc, "food_loss_on_reproduction", &food_loss_on_reproduction );

	json::get( doc, "food_amount", &food_amount );
	json::get( doc, "max_hunger", &max_hunger );
	json::get( doc, "hunger_gain_rate", &hunger_gain_rate );
	json::get( doc, "min_hunger_to_eat", &min_hunger_to_eat );

	adjectives = (Adjectives)json::get( doc, "adjectives", 0Ui32 );

	return true;
}
