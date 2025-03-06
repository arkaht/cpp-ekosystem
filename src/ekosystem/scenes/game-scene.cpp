#include "game-scene.h"

#include <suprengine/core/assets.h>

#include <suprengine/rendering/opengl/opengl-render-batch.h>

#include <suprengine/utils/random.h>

#include <suprengine/tools/vis-debug.h>

using namespace eks;

GameScene::GameScene()
{}

GameScene::~GameScene()
{
	if ( _world != nullptr )
	{
		delete _world;
	}
}

void GameScene::init()
{
	auto& engine = Engine::instance();

	auto cube_model = Assets::get_model( MESH_CUBE );
	auto cylinder_model = Assets::get_model( MESH_CYLINDER );
	auto sphere_model = Assets::get_model( MESH_SPHERE );

	setup_world();

	//  Setup camera
	constexpr float CAMERA_SPEED = 100.0f;

	auto camera_owner = engine.create_entity<Entity>();
	camera_owner->transform->location = Vec3 { _world->get_size() * 0.5f * _world->TILE_SIZE, 0.4f };
	camera_owner->transform->rotation = Quaternion( DegAngles { -45.0f, -135.0f, 0.0f } );
	_camera_controller = camera_owner->create_component<CameraController>(
		CAMERA_SPEED,
		Vec3 { 15.0f, 15.0f, 20.0f }
	);

	CameraProjectionSettings projection_settings {};
	projection_settings.fov = 60.0f;

	auto camera = camera_owner->create_component<Camera>( projection_settings );
	camera->activate();

	RenderBatch* render_batch = engine.get_render_batch();
	render_batch->set_ambient_direction( Vec3 { 0.0f, 0.0f, 1.0f } );

	//  Assign references to debug menu
	_debug_menu.camera_controller = _camera_controller;
	_debug_menu.world = _world;
}

void GameScene::setup_world()
{
	_world = new World( Vec2 { 20.0f, 20.0f } );

	auto hare_data  = _world->get_pawn_data( "hare" );
	auto grass_data = _world->get_pawn_data( "grass" );
	auto wolf_data  = _world->get_pawn_data( "wolf" );

	//	Spawn grass
	for ( int i = 0; i < 8; i++ )
	{
		_world->create_pawn( grass_data, _world->find_random_tile_pos() );
	}

	//	Spawn hares
	for ( int i = 0; i < 6; i++ )
	{
		auto hare = _world->create_pawn( hare_data, _world->find_random_tile_pos() );
		hare->group_id = 2;
	}

	//	Spawn wolves
	for ( int i = 0; i < 2; i++ )
	{
		auto wolf = _world->create_pawn( wolf_data, _world->find_random_tile_pos() );
		wolf->group_id = 1; //	Prevent wolves from eating each other
	}

	//	Set default group limits
	_world->set_group_limit( 1, 4 );	//	Wolves
	_world->set_group_limit( 2, 20 );	//	Hares
}

void GameScene::update( float dt )
{
	const Engine* engine = _game->get_engine();

	const InputManager* inputs = engine->get_inputs();

	Physics* physics = engine->get_physics();
	const Ray ray = engine->camera->viewport_to_world( inputs->get_mouse_pos() );
	const RayParams params {};

	RayHit hit {};
	bool has_hit = physics->raycast( ray, &hit, params );
#ifdef ENABLE_VISDEBUG
	if ( has_hit )
	{
		const Vec3 box_bounds {
			_world->TILE_SIZE * 0.25f,
			_world->TILE_SIZE * 0.25f,
			0.0f
		};

		const Vec3 hit_grid_location = _world->world_to_grid( hit.point );
		VisDebug::add_box(
			_world->grid_to_world( hit_grid_location ),
			Quaternion::identity,
			Box { -box_bounds, box_bounds },
			Color::white,
			0.0f,
			DebugChannel::Entity
		);
	}
#endif

	//  Left Click: Spawn a pawn where we click in the world via a raycast
	if ( inputs->is_mouse_button_just_pressed( MouseButton::Left ) )
	{
		if ( has_hit )
		{
			_debug_menu.create_pawn(
				_world->get_pawn_data( _debug_menu.get_selected_pawn_data_name() ),
				_world->world_to_grid( hit.point )
			);

		#ifdef ENABLE_VISDEBUG
			VisDebug::add_line( ray.origin, hit.point, Color::green, 5.0f, DebugChannel::Camera );
			VisDebug::add_sphere( hit.point, 0.5f, Color::red, 5.0f, DebugChannel::Camera );
		#endif
		}
		else
		{
		#ifdef ENABLE_VISDEBUG
			VisDebug::add_line( ray.origin, ray.get_end_point(), Color::red, 5.0f, DebugChannel::Camera );
			VisDebug::add_box(
				ray.origin + ray.direction * 10.0f,
				Quaternion::look_at( ray.direction, Vec3::up ),
				Box::one * 0.1f,
				Color::red,
				5.0f,
				DebugChannel::Camera
			);
		#endif
		}
	}
}
