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

	_world->create_pawn( hare_data, _world->find_random_tile_pos() );
	_world->create_pawn( hare_data, _world->find_random_tile_pos() );
	_world->create_pawn( hare_data, _world->find_random_tile_pos() );
	_world->create_pawn( wolf_data, _world->find_random_tile_pos() );
	_world->create_pawn( grass_data, _world->find_random_tile_pos() );
	_world->create_pawn( grass_data, _world->find_random_tile_pos() );
	_world->create_pawn( grass_data, _world->find_random_tile_pos() );
	_world->create_pawn( grass_data, _world->find_random_tile_pos() );
}

void GameScene::update( float dt )
{
	Engine* engine = _game->get_engine();
	float time = engine->get_updater()->get_accumulated_seconds();

	InputManager* inputs = engine->get_inputs();

	//  Left Click: Spawn a pawn where we click in the world via a raycast
	if ( inputs->is_mouse_button_just_pressed( MouseButton::Left ) )
	{
		Physics* physics = engine->get_physics();

		Ray ray = engine->camera->viewport_to_world( inputs->get_mouse_pos() );

		RayParams params {};

		RayHit hit {};
		bool has_hit = physics->raycast( ray, &hit, params );
		if ( has_hit )
		{
			_debug_menu.create_pawn(
				_world->get_pawn_data( _debug_menu.get_selected_pawn_data_name() ),
				_world->world_to_grid( hit.point )
			);

			VisDebug::add_line( ray.origin, hit.point, Color::green, 5.0f, DebugChannel::Camera );
			VisDebug::add_sphere( hit.point, 0.5f, Color::red, 5.0f, DebugChannel::Camera );
		}
		else
		{
			VisDebug::add_line( ray.origin, ray.get_end_point(), Color::red, 5.0f, DebugChannel::Camera );
			VisDebug::add_box(
				ray.origin + ray.direction * 10.0f,
				Quaternion::look_at( ray.direction, Vec3::up ),
				Box::one * 0.1f,
				Color::red,
				5.0f,
				DebugChannel::Camera
			);
		}
	}
}
