#pragma once

#include <suprengine/usings.h>
#include <suprengine/model.h>
#include <suprengine/enum-flags.hpp>

namespace eks
{
	using namespace suprengine;

	enum class Adjectives : uint32_t
	{
		None			= 1 << 0,

		//  Food consumption

		//  Consume light as food.
		Photosynthesis	= 1 << 1,
		//  Consume meat as food.
		Carnivore		= 1 << 2,
		//  Consume vegetal as food.
		Herbivore		= 1 << 3,

		//  Food type
		
		//  Is eatable by carnivores.
		Meat			= 1 << 4,
		//  Is eatable by herbivore.
		Vegetal			= 1 << 5,
	};
	DEFINE_ENUM_WITH_FLAGS( Adjectives, uint32_t )

	struct PawnData
	{
		//  "Food" is the opposite of "hunger": 1.0f - hunger

		//  Unique name of the pawn data
		std::string name = "N/A";
		//  Model of the pawn
		SharedPtr<Model> model;
		//  Color of the pawn
		Color modulate;

		//  In tile per seconds, how fast the pawn is?
		float move_speed = 1.0f;

		//  How much child does it generate upon reproduction?
		//  Set to 0 to disable reproduction.
		int child_spawn_count = 0;
		//  Minimum amount of hunger this pawn needs before 
		//  reproducing
		float min_food_reproduction = 0.8f;
		//  Amount of food to lose after reproduction
		float food_loss_on_reproduction = 0.4f;

		//  Amount of food this pawn provide when eaten
		float food_amount = 1.0f;
		//  Maximum amount of hunger this pawn can hold
		float max_hunger = 1.0f;
		//  Rate of increase of hunger per second
		float hunger_gain_rate = 0.1f;
		//  Minimum amount of hunger to start eating
		float min_hunger_to_eat = 0.4f;

		//  Behaviors defining this pawn
		Adjectives adjectives = Adjectives::None;

		/*
		 * Returns whenever the data has a given adjective.
		 */
		bool has_adjective( Adjectives adjective ) const
		{
			return ( adjectives & adjective ) == adjective;
		}
	};
}