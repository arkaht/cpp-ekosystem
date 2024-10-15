#include "pawn.h"

#include <suprengine/assets.h>
#include <suprengine/random.h>

#include "states/pawn-wander.h"
#include "states/pawn-reproduction.h"
#include "states/pawn-chase.h"

using namespace eks;

Pawn::Pawn( World* world, SafePtr<PawnData> _data )
	: _world( world ), data( _data )
{
	//  Avoid immediate reproduction upon creation
	hunger = 1.0f - data->hunger_consumption_on_reproduction;
}

void Pawn::setup()
{
	auto model = Assets::get_model( data->model_name );

	_renderer = create_component<ModelRenderer>( 
		model, 
		SHADER_LIT_MESH, 
		data->modulate
	);

	_state_machine = create_component<StateMachine<Pawn>>();
	_state_machine->create_state<PawnChaseState>();
	_state_machine->create_state<PawnReproductionState>();
	_state_machine->create_state<PawnWanderState>();
	_state_machine->is_active = false;	//	Disable updates by the engine for manual updates

	//	Loading pawn's assets
	if ( !data->movement_height_curve_name.empty() )
	{
		movement_height_curve = Assets::get_curve( data->movement_height_curve_name );
	}
}

void Pawn::update_this( float dt )
{
	//  TODO: Debug build only
	_renderer->modulate = data->modulate;

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

void Pawn::tick( float dt )
{
	//	Manually updating the state machine using the substepping tick
	//	of the pawn
	_state_machine->update( dt );

	//  Hunger gain
	hunger = math::max(
		hunger - data->natural_hunger_consumption * dt,
		0.0f
	);

	//  Photosynthesis
	if ( data->has_adjective( Adjectives::Photosynthesis ) )
	{
		hunger = math::min( 
			hunger + data->photosynthesis_gain * dt,
			data->max_hunger
		);
	}

	//  Kill from hunger
	if ( hunger <= 0.0f )
	{
		kill();
	}
}

void Pawn::reproduce( SafePtr<Pawn> partner )
{
	//	Generate children around
	int child_spawn_count = random::generate( data->min_child_spawn_count, data->max_child_spawn_count );
	for ( int i = 0; i < child_spawn_count; i++ )
	{
		Vec3 spawn_pos;
		if ( !_world->find_empty_tile_pos_around( _tile_pos, &spawn_pos ) ) continue;

		auto child = _world->create_pawn( data, spawn_pos );
		child->group_id = group_id;
	}

	//	Consume hunger
	hunger -= data->hunger_consumption_on_reproduction;

	//	Consume partner's hunger
	if ( partner.is_valid() )
	{
		partner->hunger -= partner->data->hunger_consumption_on_reproduction;
	}
}

void Pawn::set_tile_pos( const Vec3& tile_pos )
{
	transform->location = tile_pos * _world->TILE_SIZE;
	update_tile_pos();
}

void Pawn::update_tile_pos()
{
	_tile_pos = Vec3::world_to_grid( transform->location, _world->TILE_SIZE );
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

std::string Pawn::get_name() const
{
	return data->name + "#" + std::to_string( get_unique_id() );
}

World* Pawn::get_world() const
{
	return _world;
}