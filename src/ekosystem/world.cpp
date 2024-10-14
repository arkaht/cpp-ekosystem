#include "world.h"

#include <suprengine/assert.hpp>
#include <suprengine/assets.h>
#include <suprengine/components/colliders/box-collider.h>
#include <suprengine/random.h>
#include <suprengine/json.h>

#include "entities/pawn.h"

#include <filesystem>

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

void World::add_pawn_data( SharedPtr<PawnData> data )
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

bool World::find_empty_tile_pos_around( const Vec3& pos, Vec3* out ) const
{
	//  Randomize signs to avoid giving the same direction each time
	int random_sign_x = random::generate_sign();
	int random_sign_y = random::generate_sign();

	for ( int x = -1; x <= 1; x++ )
	{
		for ( int y = -1; y <= 1; y++ )
		{
			//  Filter out input position
			if ( x == 0 && y == 0 ) continue;

			out->x = pos.x + x * random_sign_x;
			out->y = pos.y + y * random_sign_y;

			//  Filter out any out-of-bounds positions
			if ( out->x < 0 || out->x > _size.x ) continue;
			if ( out->y < 0 || out->y > _size.y ) continue;

			//  Filter out position already containing a pawn
			auto pawn = find_pawn_at( Adjectives::None, *out );
			if ( pawn.is_valid() ) continue;

			return true;
		}
	}

	return false;
}

Vec3 World::find_random_tile_pos() const
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
) const
{
	for ( auto& pawn : _pawns )
	{
		if ( pawn == pawn_to_ignore ) continue;
		if ( !pawn.is_valid() ) continue;
		if ( !pawn->data->has_adjective( adjectives ) ) continue;

		return pawn;
	}

	return nullptr;
}

SafePtr<Pawn> World::find_pawn_at(
	Adjectives adjectives,
	const Vec3& pos
) const
{
	for ( auto& pawn : _pawns )
	{
		if ( !pawn.is_valid() ) continue;
		if ( pawn->get_tile_pos() != pos ) continue;
		if ( !pawn->data->has_adjective( adjectives ) ) continue;

		return pawn;
	}

	return nullptr;
}

SafePtr<Pawn> eks::World::find_nearest_pawn(
	const Vec3& origin,
	std::function<bool( SafePtr<Pawn> )> callback
) const
{
	SafePtr<Pawn> nearest_pawn = nullptr;
	float nearest_dist = math::PLUS_INFINITY;
	for ( auto& pawn : _pawns )
	{
		if ( callback( pawn ) )
		{
			float dist = Vec3::distance2d_sqr(
				origin,
				pawn->get_tile_pos()
			);

			if ( nearest_pawn == nullptr || nearest_dist > dist )
			{
				nearest_pawn = pawn;
				nearest_dist = dist;
			}
		}
	}

	return nearest_pawn;
}

SafePtr<Pawn> World::find_pawn( std::function<bool( SafePtr<Pawn> )> callback ) const
{
	for ( auto& pawn : _pawns )
	{
		if ( callback( pawn ) )
		{
			return pawn;
		}
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

std::map<std::string, SharedPtr<PawnData>>& World::get_pawn_datas()
{
	return _pawn_datas;
}

Vec2 World::get_size() const
{
	return _size;
}

void World::_init_datas()
{
	//  Load all pawn data files
	std::filesystem::directory_iterator itr( "assets/ekosystem/data/pawns/" );
	for ( const auto& entry : itr )
	{
		if ( entry.is_directory() ) continue;

		auto& file_path = entry.path();
		Logger::info( "Loading pawn data at '%s'", file_path.string().c_str() );

		//  Read file contents
		std::ifstream file( file_path );
		std::string content(
			( std::istreambuf_iterator<char>( file ) ),
			( std::istreambuf_iterator<char>() )
		);
		file.close();

		//  Parse contents into JSON
		json::document doc {};
		doc.Parse( content.c_str() );

		//  Unserialize JSON to game data
		auto data = std::make_shared<PawnData>();
		data->name = file_path.filename().replace_extension().string();
		data->unserialize( doc );
		add_pawn_data( data );
	}
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
