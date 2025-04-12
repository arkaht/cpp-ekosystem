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

	setup_world();

	//  Setup camera
	auto camera_owner = engine.create_entity<Entity>();
	camera_owner->transform->location = Vec3 { _world->get_size() * 0.5f * _world->TILE_SIZE, 0.4f };
	camera_owner->transform->rotation = Quaternion( DegAngles { -45.0f, -135.0f, 0.0f } );
	_camera_controller = camera_owner->create_component<CameraController>(
		100.0f,
		Vec3 { 15.0f, 15.0f, 20.0f }
	);

	CameraProjectionSettings projection_settings {};
	projection_settings.fov = 60.0f;
	projection_settings.znear = 0.1f;
	projection_settings.zfar = 2000.0f;

	auto camera = camera_owner->create_component<Camera>( projection_settings );
	camera->activate();

	RenderBatch* render_batch = engine.get_render_batch();
	render_batch->set_ambient_direction( Vec3 { 0.0f, 0.0f, -1.0f } );

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
	for ( int i = 0; i < 16; i++ )
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
	_world->update( dt );

	Engine* engine = _game->get_engine();

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

	//	Numerical keys: Time scale modifiers
	constexpr SDL_Scancode TIME_SCALE_SCANCODES[]
	{
		SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4
	};
	int index = 0;
	for ( SDL_Scancode scancode : TIME_SCALE_SCANCODES )
	{
		if ( inputs->is_key_just_pressed( scancode ) )
		{
			float scale_modifier = static_cast<float>( 1 << index );
			engine->get_updater()->time_scale = inputs->is_key_down( SDL_SCANCODE_LSHIFT ) ? 1.0f / scale_modifier : scale_modifier;
			break;
		}
		index++;
	}
	
	//	Space: Pausing/Un-pausing game
	if ( inputs->is_key_just_pressed( SDL_SCANCODE_SPACE ) )
	{
		engine->is_game_paused = !engine->is_game_paused;
	}

	RenderBatch* render_batch = engine->get_render_batch();

#ifdef ENABLE_VISDEBUG
	if ( VisDebug::is_channel_active( DebugChannel::Lighting ) )
	{
		AmbientLightInfos ambient_infos = render_batch->get_ambient_infos();
		Vec3 ambient_origin { 0.0f, 0.0f, 10.0f };

		VisDebug::add_box( ambient_origin, Quaternion::identity, Box::half, Color::white, 0.0f, DebugChannel::Lighting );
		VisDebug::add_line( ambient_origin, ambient_origin + ambient_infos.direction * 5.0f, Color::white, 0.0f, DebugChannel::Lighting );
	}
#endif

	//	Animate grass shader
	if ( SharedPtr<Shader> grass_shader = Assets::get_shader( "ekosystem::grass" ) )
	{
		const float game_time = engine->get_updater()->get_accumulated_seconds();
		grass_shader->activate();
		grass_shader->set_float( "u_time", game_time );
	}
}
