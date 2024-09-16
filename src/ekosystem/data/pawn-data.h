#pragma once

#include <suprengine/usings.h>
#include <suprengine/model.h>
#include <suprengine/enum-flags.hpp>
#include <suprengine/json.h>
#include <suprengine/render-batch.h>

namespace eks
{
	using namespace suprengine;

	enum class Adjectives : uint32_t
	{
		None			= 0,

		//  Food consumption

		//  Consume light as food
		Photosynthesis	= 1 << 0,
		//  Consume Meat as food
		Carnivore		= 1 << 1,
		//  Consume Vegetal as food
		Herbivore		= 1 << 2,

		//  Food type
		
		//  Is eatable by Carnivore
		Meat			= 1 << 3,
		//  Is eatable by Herbivore
		Vegetal			= 1 << 4,

		All				= 0xFFFFFFFF,
	};
	DEFINE_ENUM_WITH_FLAGS( Adjectives, uint32_t )

	struct PawnData
	{
		//  Unique name of the pawn data
		std::string name = "N/A";
		//  Model name of the pawn
		std::string model_name = MESH_CUBE;
		//  Color of the pawn
		Color modulate = Color::white;

		//  Movement speed in tile per seconds
		float move_speed = 1.0f;

		//  Range amount of child to generate upon reproduction.
		//  Set to 0 to disable reproduction.
		int min_child_spawn_count = 0;
		int max_child_spawn_count = 0;
		//  Minimum amount of hunger this pawn needs before 
		//  reproducing
		float min_hunger_for_reproduction = 0.8f;
		//  Amount of hunger to consume after reproduction
		float hunger_consumption_on_reproduction = 0.4f;

		//  Amount of food this pawn provide when eaten
		float food_amount = 1.0f;
		//  Maximum amount of hunger this pawn can hold
		float max_hunger = 1.0f;
		//  Rate of decrease of hunger per second
		float natural_hunger_consumption = 0.1f;
		//  Minimum amount of hunger to start eating
		float min_hunger_to_eat = 0.4f;

		//  Rate of increase of hunger per second by photosynthesis
		float photosynthesis_gain = 0.05f;

		//  Behaviors defining this pawn
		Adjectives adjectives = Adjectives::None;

		/*
		 * Returns whenever the data has a given adjective.
		 */
		bool has_adjective( Adjectives adjective ) const
		{
			return ( adjectives & adjective ) == adjective;
		}

		bool serialize( json::document& doc ) const;
		bool unserialize( const json::document& doc );
	};
}