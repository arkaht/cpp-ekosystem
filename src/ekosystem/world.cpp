#include "world.h"

#include <suprengine/core/assets.h>
#include <suprengine/core/engine.h>

#include <suprengine/components/colliders/box-collider.h>
#include <suprengine/components/renderers/model-renderer.hpp>

#include <suprengine/utils/assert.h>
#include <suprengine/utils/random.h>
#include <suprengine/utils/json.h>

#include "entities/pawn.h"
#include "components/particle-renderer.h"

#include <filesystem>

using namespace eks;

World::World( const Vec2& size )
{
	auto& engine = Engine::instance();
	auto model = Assets::get_model( "ekosystem::floor" );

	//  Setup ground
	_ground = engine.create_entity<Entity>();
	_ground->transform->location = Vec3 { 0.0f, 0.0f, -1.0f };
	_ground_renderer = _ground->create_component<ModelRenderer>( model, SHADER_LIT_MESH );
	_ground->create_component<BoxCollider>( Box::one );

	_sun = engine.create_entity<Entity>();
	_sun->create_component<ModelRenderer>( Assets::get_model( "suprengine::sphere" ), "suprengine::color", Color::from_hex( "#fbff00" ) );
	_sun->transform->scale = Vec3( 50.0f );

	_moon = engine.create_entity<Entity>();
	_moon->create_component<ModelRenderer>( Assets::get_model( "suprengine::sphere" ), "suprengine::color", Color::from_hex( "#ffffff" ) );
	_moon->transform->scale = Vec3( 50.0f );
	
	resize( size );

	_init_datas();

	engine.on_entity_removed.listen( &World::_on_entity_removed, this );
}

World::~World()
{
	clear();

	if ( _ground.is_valid() )
	{
		_ground->kill();
	}
}

void World::update( float dt )
{
	// Update world time
	constexpr float FULL_CYCLE_GAME_TIME = 24.0f;
	_world_time = math::fmod( _world_time + dt, FULL_CYCLE_GAME_TIME );

	// Update sun direction
	const float sun_angle = math::HALF_PI + _world_time / FULL_CYCLE_GAME_TIME * math::DOUBLE_PI;
	_sun_direction.x = math::cos( sun_angle );
	_sun_direction.z = math::sin( sun_angle );
	_photosynthesis_multiplier = math::max( 0.0f, Vec3::dot( _sun_direction, -Vec3::up ) );

	_sun->transform->set_location( -_sun_direction * 500.0f );
	_moon->transform->set_location( _sun_direction * 500.0f );

	//printf( "WorldTime=%.2f: PhMul=%.2f\n", _world_time, get_photosynthesis_multiplier() );

	const bool is_sun_time = Vec3::dot( _sun_direction, -Vec3::up ) > 0.0f;

	// Update ambient direction
	Engine& engine = Engine::instance();
	RenderBatch* render_batch = engine.get_render_batch();
	render_batch->set_ambient_direction( is_sun_time ? _sun_direction : -_sun_direction );
	render_batch->set_ambient_color( is_sun_time ? Color::from_hex( "#fff98a" ) : Color::from_hex( "#666666" ) );
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
		Logger::critical(
			"A pawn data with 'name' property equals to '%s' already exists! Please ensure they all are unique!",
			*data->name
		);
		return;
	}

	_pawn_datas[data->name] = data;
	Logger::info(
		"A pawn data has been registered as '%s'",
		*data->name
	);
}

SafePtr<PawnData> World::get_pawn_data( rconst_str name ) const
{
	return _pawn_datas.at( name );
}

void World::set_group_limit( GroupID group_id, uint8 limit )
{
	ASSERT( group_id >= 0 && group_id <= MAX_PAWN_GROUP_ID );
	_group_limits[group_id] = limit;
}

int World::get_group_limit( GroupID group_id ) const
{
	ASSERT( group_id >= 0 && group_id <= MAX_PAWN_GROUP_ID );
	return _group_limits[group_id];
}

int World::get_pawns_count_in_group( GroupID group_id ) const
{
	int count = 0;

	for ( const SafePtr<Pawn>& pawn : _pawns )
	{
		if ( pawn->group_id == group_id )
		{
			count++;
		}
	}

	return count;
}

void World::resize( const Vec2& size )
{
	_size = size;

	_ground->transform->set_scale(
		Vec3 {
			TILE_SIZE * 0.5f + _size.x * TILE_SIZE * 0.5f,
			TILE_SIZE * 0.5f + _size.y * TILE_SIZE * 0.5f,
			1.0f
		}
	);

	_ground_renderer->model->get_mesh( 0 )->tiling = _size;
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

bool World::find_empty_tile_pos_around( const Vec3& pos, Vec3* out, Adjectives adjectives_filter ) const
{
	//  Randomize signs to avoid giving the same direction each time
	int random_sign_x = random::generate_sign();
	int random_sign_y = random::generate_sign();

	const Box bounds = get_bounds();
	for ( int x = -1; x <= 1; x++ )
	{
		for ( int y = -1; y <= 1; y++ )
		{
			//  Filter out input position
			if ( x == 0 && y == 0 ) continue;

			out->x = pos.x + x * random_sign_x;
			out->y = pos.y + y * random_sign_y;

			//  Filter out any out-of-bounds positions
			if ( out->x < bounds.min.x || out->x > bounds.max.x ) continue;
			if ( out->y < bounds.min.y || out->y > bounds.max.y ) continue;

			//  Filter out position already containing a pawn
			SafePtr<Pawn> pawn = nullptr;
			if ( adjectives_filter == Adjectives::None )
			{
				pawn = find_pawn_at( Adjectives::None, *out );
			}
			else
			{
				pawn = find_pawn( [&]( SafePtr<Pawn> pawn ) {
					if ( pawn->get_tile_pos() != *out ) return false;
					if ( pawn->data->has_adjective( adjectives_filter ) ) return false;
					return true;
				} );
			}

			if ( pawn.is_valid() ) continue;

			return true;
		}
	}

	return false;
}

Vec3 World::find_random_tile_pos() const
{
	const Box bounds = get_bounds();
	return Vec3::snap_to_grid( random::generate_location( bounds ), 1.0f );
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

Vec3 World::world_to_grid( const Vec3& world_pos ) const
{
	const Vec3 offset = Vec3 {
		TILE_SIZE * 0.5f,
		TILE_SIZE * 0.5f,
		0.0f
	};

	Vec3 grid_pos = Vec3::world_to_grid( world_pos + offset, TILE_SIZE );
	grid_pos.z = 0.0f;
	return grid_pos;
}

Vec3 World::grid_to_world( const Vec3& grid_pos ) const
{
	return grid_pos * TILE_SIZE;
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

Box World::get_bounds() const
{
	//	TODO: Fix the callers using this function giving weird results with an odd-sized world
	const Vec3 half_size( _size * 0.5f, 0.0f );
	return Box { -half_size, half_size };
}

Vec3 World::get_sun_direction() const
{
	return _sun_direction;
}

float World::get_photosynthesis_multiplier() const
{
	return _photosynthesis_multiplier;
}

bool World::is_within_world_time( float min_hours, float max_hours ) const
{
	// "Are we between 8am and 4am?" is the same as: "Are we not between 4am and 8am?"
	if ( min_hours > max_hours )
	{
		return !( max_hours < _world_time && _world_time < min_hours );
	}

	return min_hours < _world_time && _world_time < max_hours;
}

float World::get_world_time() const
{
	return _world_time;
}

void World::_init_datas()
{
	// Sleep particle system
	SharedPtr<ParticleSystemData> sleep_particle_system = std::make_shared<ParticleSystemData>( 
		ParticleSystemData {
			.mesh = Assets::get_model( "ekosystem::facing.plane" )->get_mesh( 0 ),
			.shader = Assets::get_shader( "suprengine::texture" ),
			.texture = Assets::get_texture( "ekosystem::icon.sleep" ),
			.render_scale = Vec3( 1.0f ),
			.start_velocity = Vec3 { 1.5f, -1.5f, 1.5f },
			.spawn_location_offset = Vec3 { 0.0f, 0.0f, 5.0f },
			.custom_updater = [&]( ParticleInstance* particle, ParticleRenderer* renderer, float dt )
			{
				const SharedPtr<Curve> alpha_curve = Assets::get_curve( "particles/sleep-alpha" );
				const SharedPtr<Curve> scale_curve = Assets::get_curve( "particles/sleep-scale" );

				//	Offset
				particle->offset.x = 0.08f * math::sin( ( particle->unique_id * 2.0f + particle->lifetime ) * 3.0f );
				particle->offset.y = -0.16f * math::cos( ( particle->unique_id * 2.0f + particle->lifetime ) * 3.0f );

				//	Modulate & Scale
				const float time = particle->lifetime / renderer->system_data->max_lifetime;
				particle->modulate.a = static_cast<uint8>( alpha_curve->evaluate_by_time( time ) * 255.0f );
				particle->scale = scale_curve->evaluate_by_time( time );
			},
			.spawn_rate = 1.0f,
			.max_lifetime = 3.0f,
		}
	);
	// Love particle system
	SharedPtr<ParticleSystemData> love_particle_system = std::make_shared<ParticleSystemData>( 
		ParticleSystemData {
			.mesh = Assets::get_model( "ekosystem::facing.plane" )->get_mesh( 0 ),
			.shader = Assets::get_shader( "suprengine::texture" ),
			.texture = Assets::get_texture( "ekosystem::icon.love" ),
			.render_scale = Vec3( 1.0f ),
			.velocity_loss = Vec3 { 3.0f, 3.0f, 1.5f },
			.spawn_location_offset = Vec3 { 0.0f, 0.0f, 3.0f },
			.custom_creator = [&]( ParticleInstance* particle, ParticleRenderer* renderer )
			{
				particle->velocity = Vec3::transform( Vec3::forward, random::generate_rotation() ) * 25.0f;
				if ( particle->velocity.z < 0.0f )
				{
					particle->velocity.z = -particle->velocity.z;
				}
			},
			.custom_updater = [&]( ParticleInstance* particle, ParticleRenderer* renderer, float dt )
			{
				const SharedPtr<Curve> alpha_curve = Assets::get_curve( "particles/sleep-alpha" );
				const SharedPtr<Curve> scale_curve = Assets::get_curve( "particles/sleep-scale" );

				//	Offset
				particle->offset.x = 0.3f * math::sin( ( particle->unique_id * 2.0f + particle->lifetime ) * 3.0f );
				particle->offset.y = -0.5f * math::cos( ( particle->unique_id * 2.0f + particle->lifetime ) * 3.0f );

				//	Modulate & Scale
				const float time = particle->lifetime / renderer->system_data->max_lifetime;
				particle->modulate.a = static_cast<uint8>( alpha_curve->evaluate_by_time( time ) * 255.0f );
				particle->scale = scale_curve->evaluate_by_time( time );
			},
			.is_one_shot = true,
			.spawn_rate = 10.0f,
			.max_lifetime = 3.0f,
		}
	);

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
		data->sleep_particle_system = sleep_particle_system;
		data->love_particle_system = love_particle_system;
		data->unserialize( doc );
		add_pawn_data( data );
	}
}

void World::_on_entity_removed( Entity* entity )
{
	if ( auto pawn = entity->cast<Pawn>() )
	{
		auto itr = std::find( _pawns.begin(), _pawns.end(), SafePtr<Pawn>( pawn ) );
		ASSERT_MSG( itr != _pawns.end(), "A removed pawn couldn't be erased from the World pawns list!" );

		_pawns.erase( itr );

		printf( "Pawn '%s' is being removed!\n", pawn->get_name().c_str() );
	}
}
