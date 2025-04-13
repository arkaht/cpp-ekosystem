#include "pawn.h"

#include <suprengine/core/engine.h>
#include <suprengine/core/assets.h>
#include <suprengine/utils/random.h>

#include <ekosystem/components/particle-renderer.h>

#include "states/pawn-flee.h"
#include "states/pawn-chase.h"
#include "states/pawn-reproduction.h"
#include "states/pawn-sleep.h"
#include "states/pawn-wander.h"

using namespace eks;

Pawn::Pawn( World* world, SafePtr<PawnData> data )
	: _world( world ), data( data ), _name( data->name + "#" + std::to_string( get_unique_id() ) )
{
	hunger = data->hunger_at_spawn;
}

void Pawn::setup()
{
	auto model = Assets::get_model( data->model_name );

	_renderer = create_component<ModelRenderer>( 
		model,
		data->shader_name,
		data->modulate
	);

	if ( data->move_speed > 0.0f )
	{
		_sleep_particle_renderer = create_component<ParticleRenderer>();
		_sleep_particle_renderer->is_spawning = false;
		_sleep_particle_renderer->system_data = data->sleep_particle_system;

		_love_particle_renderer = create_component<ParticleRenderer>();
		_love_particle_renderer->is_spawning = false;
		_love_particle_renderer->system_data = data->love_particle_system;

		//	Do not create a state machine for pawns with no ability to move.
		//	It greatly helps to optimize memory usage and CPU time (e.g. I have
		//	reduced over 700KiB of RAM by not creating 1 state machine component,
		//	4 states and 11 tasks for each Grass pawn) since they do not use it.
		_state_machine = create_component<StateMachine<Pawn>>();
		_state_machine->create_state<PawnFleeState>( 4.0f );
		_state_machine->create_state<PawnChaseState>();
		_state_machine->create_state<PawnSleepState>();
		_state_machine->create_state<PawnReproductionState>( this );
		_state_machine->create_state<PawnWanderState>();
		_state_machine->is_active = false;	//	Disable updates by the engine for manual updates
	}
}

void Pawn::update_this( float dt )
{
	//  TODO: Debug build only
	_renderer->modulate = data->modulate;

	if ( _sleep_particle_renderer )
	{
		//	TODO: Fix previous particles still alive when resuming activating
		_sleep_particle_renderer->is_spawning = is_sleeping;
		_sleep_particle_renderer->play_rate = math::lerp(
			_sleep_particle_renderer->play_rate,
			is_sleeping ? 1.0f : 4.0f,
			dt * 4.0f
		);
	}

	if ( dt > 0.0f )
	{
		//  Substepping tick
		//  NOTE: It fixes issues in behaviors when the time scale is 
		//	really high (e.g. time scale set to 32). Reproduction could
		//  not happen if the threshold was too thin.
		auto& engine = Engine::instance();
		int substeps = (int)math::ceil( engine.get_updater()->time_scale );
		float subdelta = dt / substeps;
		for ( int substep = 0; substep < substeps; substep++ )
		{
			tick( subdelta );
		}
	}
}

void Pawn::tick( float dt )
{
	//	Manually update the state machine using the substepping tick
	//	of the pawn
	if ( _state_machine != nullptr )
	{
		_state_machine->update( dt );
	}

	//  Hunger gain
	float hunger_modifier = is_sleeping ? _world->pawn_hunger_sleep_modifier : 1.0f;
	if ( hunger_modifier > 0.0f )
	{
		hunger = math::max(
			hunger - data->natural_hunger_consumption * hunger_modifier * dt,
			0.0f
		);
	}

	//  Photosynthesis
	if ( data->has_adjective( Adjectives::Photosynthesis ) )
	{
		hunger = math::min( 
			hunger + data->photosynthesis_gain * _world->get_photosynthesis_multiplier() * dt,
			data->max_hunger
		);

		//	Manual reproduction for photosynthesis pawns without a state machine
		if ( _state_machine == nullptr && hunger >= data->min_hunger_for_reproduction )
		{
			reproduce( nullptr );
		}
	}

	//  Kill from hunger
	if ( hunger <= 0.0f )
	{
		kill();
	}
}

void Pawn::reproduce( SafePtr<Pawn> partner )
{
	//	Get the number of children to born
	int child_spawn_count = random::generate( data->min_child_spawn_count, data->max_child_spawn_count );

	//	Prevent from giving birth to a number of children that it exceeds the population limit
	const World* world = get_world();
	const int population_limit = world->get_group_limit( group_id );
	if ( population_limit > 0 )
	{
		const int current_population = world->get_pawns_count_in_group( group_id );
		if ( current_population + child_spawn_count > population_limit )
		{
			child_spawn_count = population_limit - current_population;
		}
	}

	ASSERT( child_spawn_count > 0 );

	//	Generate children around
	//	NOTE: We only want animals to be able to spawn on vegetal.
	const Adjectives empty_adjectives_filter = data->has_adjective( Adjectives::Vegetal )
											 ? Adjectives::None
											 : Adjectives::Vegetal;
	Vec3 spawn_pos;
	int spawned_children_count = 0;
	for ( int i = 0; i < child_spawn_count; i++ )
	{
		if ( !_world->find_empty_tile_pos_around( _tile_pos, &spawn_pos, empty_adjectives_filter ) ) continue;

		auto child = _world->create_pawn( data, spawn_pos );
		child->group_id = group_id;
		spawned_children_count++;
	}

	//	Consume hunger
	hunger -= data->hunger_consumption_on_reproduction;
	partner_pawn = nullptr;

	//	Consume partner's hunger
	if ( partner.is_valid() )
	{
		partner->hunger -= partner->data->hunger_consumption_on_reproduction;
		partner->partner_pawn = nullptr;
		Logger::info(
			"%s gave birth to %d/%d children by mating with %s.",
			*get_name(),
			spawned_children_count, child_spawn_count,
			*partner->get_name()
		);
	}

	//	Spawn love particles
	if ( _love_particle_renderer != nullptr )
	{
		_love_particle_renderer->spawn_particles();
	}
}

void Pawn::set_tile_pos( const Vec3& tile_pos )
{
	transform->location = tile_pos * _world->TILE_SIZE;
	update_tile_pos();
}

void Pawn::update_tile_pos()
{
	_tile_pos = _world->world_to_grid( transform->location );
}

Vec3 Pawn::get_tile_pos() const
{
	return _tile_pos;
}

bool Pawn::can_reproduce() const
{
	return data->max_child_spawn_count > 0
		&& hunger >= data->min_hunger_for_reproduction;
}

bool Pawn::is_same_group( GroupID target_group_id ) const
{
	return group_id > 0 && group_id == target_group_id;
}

const std::string& Pawn::get_name() const
{
	return _name;
}

World* Pawn::get_world() const
{
	return _world;
}

SafePtr<StateMachine<Pawn>> Pawn::get_state_machine() const
{
	return _state_machine;
}
