#include "pawn.h"

#include <suprengine/assets.h>
#include <suprengine/random.h>

#include "states/pawn-chase-state.h"

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
	_state_machine->switch_state( _state_machine->create_state<PawnChaseState>() );

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
	//bool has_just_reached_tile = false;
	//if ( _move_path.size() > 0 )
	//{
	//	const Vec3 next_tile = _move_path.at( 0 );

	//	_move_progress = math::min( 
	//		_move_progress + data->move_speed * dt, 
	//		1.0f 
	//	);

	//	Vec3 new_tile_pos = Vec3::lerp( 
	//		_tile_pos,
	//		next_tile,
	//		_move_progress
	//	);
	//	if ( _move_progress >= 1.0f )
	//	{
	//		_tile_pos = new_tile_pos;

	//		_move_progress = 0.0f;
	//		//  TODO: Refactor to erase from end
	//		_move_path.erase( _move_path.begin() );

	//		has_just_reached_tile = true;
	//		//printf( "end node %d\n", (int)_move_path.size() );
	//	}
	//	//printf( "%s\n", new_tile_pos.to_string().c_str() );

	//	Vec3 render_pos = new_tile_pos * _world->TILE_SIZE;
	//	if ( _movement_height_curve )
	//	{
	//		render_pos.z = _movement_height_curve->evaluate_by_time( _move_progress );
	//	}
	//	transform->set_location( render_pos );
	//}

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

	//if ( !data->has_adjective( Adjectives::Photosynthesis )
	//  && hunger < data->min_hunger_to_eat )
	//{
	//	if ( !_food_target.is_valid() )
	//	{
	//		_move_path.clear();
	//		_find_food();
	//	}
	//	else if ( _food_target->get_tile_pos() == _tile_pos )
	//	{
	//		hunger = math::min(
	//			hunger + _food_target->data->food_amount,
	//			data->max_hunger
	//		);
	//		printf( "'%s' ate '%s'\n", 
	//			get_name().c_str(), _food_target->get_name().c_str() );
	//		
	//		_food_target->kill();
	//		_food_target.reset();
	//	}
	//	else if ( _move_path.size() == 0
	//		  || ( has_just_reached_tile && _move_path.at( _move_path.size() - 1 ) != _food_target->get_tile_pos() ) )
	//	{
	//		move_to( _food_target->get_tile_pos() );
	//	}
	//	else
	//	{
	//		/*printf( "%s == %s (%s)\n", 
	//			_food_target->get_tile_pos().to_string().c_str(),
	//			_tile_pos.to_string().c_str(),
	//			Vec3::world_to_grid( transform->location, _world->TILE_SIZE ).to_string().c_str() );*/
	//	}
	//}
	//else if ( can_reproduce() )
	//{
	//	if ( !_partner_target.is_valid() )
	//	{
	//		_move_path.clear();
	//		_find_partner();
	//	}
	//	else if ( _partner_target->hunger < _partner_target->data->min_hunger_for_reproduction )
	//	{
	//		_partner_target.reset();
	//		_find_partner();
	//	}
	//	else if ( Vec3::distance2d( _partner_target->get_tile_pos(), _tile_pos ) <= 1.5f )
	//	{
	//		reproduce();
	//	}
	//	else if ( _move_path.size() == 0
	//		  || ( has_just_reached_tile && _move_path.at( _move_path.size() - 1 ) != _partner_target->get_tile_pos() ) )
	//	{
	//		move_to( _partner_target->get_tile_pos() );
	//	}
	//}

	//  Kill from hunger
	if ( hunger <= 0.0f )
	{
		kill();
	}
}

void Pawn::move_to( const Vec3& target )
{
	_find_path_to( target );
	/*_move_progress = 0.0f;

	_move_to = target;*/
}

void Pawn::reproduce()
{
	int child_spawn_count = random::generate( data->min_child_spawn_count, data->max_child_spawn_count );
	for ( int i = 0; i < child_spawn_count; i++ )
	{
		Vec3 spawn_pos;
		if ( !_world->find_empty_tile_pos_around( _tile_pos, &spawn_pos ) ) continue;

		auto child = _world->create_pawn( data, spawn_pos );
		child->group_id = group_id;
	}

	hunger -= data->hunger_consumption_on_reproduction;

	if ( _partner_target.is_valid() )
	{
		_partner_target->hunger -= _partner_target->data->hunger_consumption_on_reproduction;
		_partner_target.reset();
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
	//_move_to = _tile_pos;
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

void Pawn::_find_food()
{
	if ( data->has_adjective( Adjectives::Herbivore ) )
	{
		auto target = _world->find_nearest_pawn(
			_tile_pos,
			[&]( auto pawn )
			{
				if ( pawn.get() == this ) return false;
				if ( pawn->is_same_group( group_id ) ) return false;
				return pawn->data->has_adjective( Adjectives::Vegetal );
			}
		);
		if ( !target.is_valid() )
		{
			printf( "'%s' can't find any vegetal to eat!\n", 
				get_name().c_str() );
			return;
		}

		_food_target = target;
		printf( "Herbivore '%s' wants to eat '%s'!\n",
			get_name().c_str(), target->get_name().c_str() );

	}
	else if ( data->has_adjective( Adjectives::Carnivore ) )
	{
		auto target = _world->find_nearest_pawn(
			_tile_pos,
			[&]( auto pawn )
			{
				if ( pawn.get() == this ) return false;
				if ( pawn->is_same_group( group_id ) ) return false;
				return pawn->data->has_adjective( Adjectives::Meat );
			}
		);
		if ( !target.is_valid() )
		{
			printf( "'%s' can't find any meat to eat!\n", 
				get_name().c_str() );
			return;
		}

		_food_target = target;
		printf( "Carnivore '%s' wants to eat '%s'!\n",
			get_name().c_str(), target->get_name().c_str() );
	}
	else if ( data->has_adjective( Adjectives::Photosynthesis ) )
	{
		hunger = data->max_hunger;
		printf( "Photosynthesis '%s' wants to eat!\n", 
			get_name().c_str() );
	}
}

void Pawn::_find_partner()
{
	if ( data->has_adjective( Adjectives::Vegetal ) )
	{
		reproduce();
	}
	else
	{
		_partner_target = _world->find_nearest_pawn(
			_tile_pos,
			[&]( auto pawn )
			{
				if ( pawn.get() == this ) return false;
				return pawn->data == data && pawn->can_reproduce();
			}
		);
		if ( !_partner_target.is_valid() ) return;

		printf( "'%s' will mate with '%s'\n", 
			get_name().c_str(), 
			_partner_target->get_name().c_str() 
		);
	}
}

void Pawn::_find_path_to( const Vec3& target )
{
	/*_move_path.clear();

	const Vec3 location = _tile_pos;
	const Vec3 diff = target - location;

	const float x_sign = math::sign( diff.x );
	const float y_sign = math::sign( diff.y );

	for ( int off_x = 1; off_x <= (int)math::abs( diff.x ); off_x++ )
	{
		_move_path.push_back( Vec3 { 
			location.x + off_x * x_sign,
			location.y,
			location.z
		} );
	}

	for ( int off_y = 1; off_y <= (int)math::abs( diff.y ); off_y++ )
	{
		_move_path.push_back( Vec3 { 
			target.x,
			location.y + off_y * y_sign,
			location.z
		} );
	}*/
}
