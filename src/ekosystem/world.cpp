#include "world.h"

#include <suprengine/assert.hpp>
#include <suprengine/assets.h>
#include <suprengine/components/colliders/box-collider.h>
#include <suprengine/random.h>

#include "entities/pawn.h"

using namespace eks;

World::World( const Vec2& size )
	: _size( size )
{
	auto& engine = Engine::instance();
	auto model = Assets::get_model( MESH_CUBE );

	//  setup ground
	_ground = engine.create_entity<Entity>();
	_ground->transform->location = Vec3 {
		_size.x * TILE_SIZE * 0.5f,
		_size.y * TILE_SIZE * 0.5f,
		-TILE_SIZE * 0.5f
	};
	_ground->transform->scale = Vec3 {
		_size.x * TILE_SIZE * 0.5f,
		_size.y * TILE_SIZE * 0.5f,
		1.0f
	};
	_ground->create_component<ModelRenderer>( model, SHADER_LIT_MESH );
	_ground->create_component<BoxCollider>( Box::HALF );

	_init_datas();

	engine.on_entity_removed.listen( "eks_world",
		std::bind( 
			&World::_on_entity_removed, this, 
			std::placeholders::_1 
		) 
	);
}

World::~World()
{
	clear();

	if ( _ground.is_valid() )
	{
		_ground->kill();
	}
}

SharedPtr<Pawn> World::create_pawn( 
	SafePtr<PawnData> data, 
	const Vec3& tile_pos 
)
{
	auto& engine = Engine::instance();

	auto pawn = engine.create_entity<Pawn>( this, data );
	pawn->set_tile_pos( tile_pos );

	_pawns.push_back( pawn );
	return pawn;
}

SafePtr<PawnData> World::get_pawn_data( rconst_str name ) const
{
	return _pawn_datas.at( name );
}

void World::clear()
{
	for ( auto& pawn : _pawns ) 
	{
		if ( !pawn.is_valid() ) continue;

		pawn->kill();
	}
	_pawns.clear();
}

Vec3 World::find_random_tile_pos()
{
	return Vec3::snap_to_grid(
		random::generate_location(
			0.0f, 0.0f, 0.0f,
			_size.x, _size.y, 0.0f
		),
		1.0f
	);
}

SafePtr<Pawn> World::find_pawn_with( 
	Adjectives adjectives, 
	SafePtr<Pawn> pawn_to_ignore 
)
{
	for ( auto& pawn : _pawns )
	{
		if ( pawn == pawn_to_ignore ) continue;
		//  TODO: Clear reference from list as soon as pawn is destroyed
		if ( !pawn.is_valid() ) continue;
		if ( !pawn->data->has_adjective( adjectives ) ) continue;

		return pawn;
	}

	return nullptr;
}

const std::vector<SafePtr<Pawn>>& World::get_pawns() const
{
	return _pawns;
}

const std::map<std::string, SharedPtr<PawnData>>& World::get_pawn_datas() const
{
	return _pawn_datas;
}

Vec2 World::get_size() const
{ 
	return _size; 
}

void World::_init_datas()
{
	auto model = Assets::get_model( MESH_CUBE );

	//  Hare
	{
		auto data = std::make_shared<PawnData>();
		data->name = "hare";
		data->model = model;
		data->modulate = Color { 100, 100, 100, 255 };
		data->move_speed = 2.0f;
		data->food_amount = 1.0f;
		data->max_hunger = 1.0f;
		data->min_hunger_to_eat = 0.3f;
		data->hunger_gain_rate = 0.05f;
		data->child_spawn_count = 2;
		data->adjectives = Adjectives::Herbivore
						 | Adjectives::Meat;
		_add_pawn_data( data );
	}

	//  Wolf
	{
		auto data = std::make_shared<PawnData>();
		data->name = "wolf";
		data->model = model;
		data->modulate = Color { 255, 10, 100, 255 };
		data->move_speed = 1.5f;
		data->food_amount = 1.0f;
		data->max_hunger = 1.0f;
		data->min_hunger_to_eat = 0.3f;
		data->hunger_gain_rate = 0.07f;
		data->child_spawn_count = 1;
		data->adjectives = Adjectives::Carnivore
						 | Adjectives::Meat;
		_add_pawn_data( data );
	}

	//  Grass
	{
		auto data = std::make_shared<PawnData>();
		data->name = "grass";
		data->model = model;
		data->modulate = Color::green;
		data->move_speed = 0.0f;
		data->food_amount = 0.3f;
		data->max_hunger = 1.0f;
		data->hunger_gain_rate = 0.05f;
		data->child_spawn_count = 0;
		data->adjectives = Adjectives::Photosynthesis
						 | Adjectives::Vegetal;
		_add_pawn_data( data );
	}
}

void World::_add_pawn_data( SharedPtr<PawnData> data )
{
	if ( _pawn_datas.find( data->name ) != _pawn_datas.end() )
	{
		Logger::critical( "A pawn data with 'name' property equals to '%s' already exists! Please ensure they all are unique!",
			data->name.c_str() );
		return;
	}

	_pawn_datas[data->name] = data;
	Logger::info( "A pawn data has been registered as '%s'", 
		data->name.c_str() );
}

void World::_on_entity_removed( Entity* entity )
{
	if ( auto pawn = entity->cast<Pawn>() )
	{
		auto itr = std::find( _pawns.begin(), _pawns.end(), SafePtr<Pawn>( pawn ) );
		ASSERT( itr != _pawns.end(), "A removed pawn couldn't be erased from the World pawns list!" );
		
		_pawns.erase( itr );

		printf( "Pawn '%s' is being removed!\n", pawn->get_name().c_str() );
	}
}
