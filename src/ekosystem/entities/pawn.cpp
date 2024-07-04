#include "pawn.h"

using namespace eks;

Pawn::Pawn( World* world, SafePtr<PawnData> _data )
	: _world( world ), data( _data )
{}

void Pawn::setup()
{
	_renderer = create_component<ModelRenderer>( 
		data->model, 
		SHADER_LIT_MESH, 
		data->modulate
	);
}

void Pawn::update_this( float dt )
{
	bool has_just_reached_tile = false;
	if ( _move_path.size() > 0 )
	{
		const Vec3 next_tile = _move_path.at( 0 );

		_move_progress = math::min( 
			_move_progress + data->move_speed * dt, 
			1.0f 
		);

		Vec3 new_tile_pos = Vec3::lerp( 
			_tile_pos,
			next_tile,
			_move_progress
		);
		if ( _move_progress >= 1.0f )
		{
			_tile_pos = new_tile_pos;

			_move_progress = 0.0f;
			//  TODO: Refactor to erase from end
			_move_path.erase( _move_path.begin() );

			has_just_reached_tile = true;
			//printf( "end node %d\n", (int)_move_path.size() );
		}
		//printf( "%s\n", new_tile_pos.to_string().c_str() );

		transform->set_location( new_tile_pos * _world->TILE_SIZE );
	}

	//  Hunger gain
	hunger = math::min( 
		hunger + data->hunger_gain_rate * dt, 
		data->max_hunger 
	);

	if ( hunger > data->min_hunger_to_eat )
	{
		if ( !_food_target.is_valid() )
		{
			_move_path.clear();
			_find_food();
		}
		else if ( _food_target->get_tile_pos() == _tile_pos )
		{
			hunger -= _food_target->data->food_amount;
			printf( "'%s' ate '%s'\n", 
				get_name().c_str(), _food_target->get_name().c_str() );
			
			_food_target->kill();
			_food_target.reset();
		}
		else if ( _move_path.size() == 0
			  || ( has_just_reached_tile && _move_path.at( _move_path.size() - 1 ) != _food_target->get_tile_pos() ) )
		{
			move_to( _food_target->get_tile_pos() );
		}
		else
		{
			/*printf( "%s == %s (%s)\n", 
				_food_target->get_tile_pos().to_string().c_str(),
				_tile_pos.to_string().c_str(),
				Vec3::world_to_grid( transform->location, _world->TILE_SIZE ).to_string().c_str() );*/
		}
	}

	//  Kill from hunger
	if ( hunger >= data->max_hunger )
	{
		kill();
	}
}

void Pawn::move_to( const Vec3& target )
{
	_find_path_to( target );
	_move_progress = 0.0f;

	_move_to = target;
}

void Pawn::set_tile_pos( const Vec3& tile_pos )
{
	transform->location = tile_pos * _world->TILE_SIZE;
	update_tile_pos();
}

void Pawn::update_tile_pos()
{
	_tile_pos = Vec3::world_to_grid( transform->location, _world->TILE_SIZE );
	_move_to = _tile_pos;
}

Vec3 Pawn::get_tile_pos() const
{
	return _tile_pos;
}

std::string Pawn::get_name() const
{
	return data->name + "#" + std::to_string( get_unique_id() );
}

void Pawn::_find_food()
{
	if ( data->has_adjective( Adjectives::Herbivore ) )
	{
		auto target = _world->find_pawn_with( 
			Adjectives::Vegetal,
			as<Pawn>()
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
		auto target = _world->find_pawn_with( 
			Adjectives::Meat,
			as<Pawn>()
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
		hunger = 0.0f;
		printf( "Photosynthesis '%s' wants to eat!\n", 
			get_name().c_str() );
	}
}

void Pawn::_find_path_to( const Vec3& target )
{
	_move_path.clear();

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
	}
}
