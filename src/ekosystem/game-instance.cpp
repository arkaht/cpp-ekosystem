#include "game-instance.h"

#include <suprengine/core/assets.h>

#include <ekosystem/scenes/game-scene.h>

using namespace eks;

void GameInstance::load_assets()
{
	SharedPtr<Texture> texture = Assets::get_texture( TEXTURE_WHITE );

	//	Textures
	SharedPtr<Texture> dirt_texture = Assets::load_texture(
		"ekosystem::dirt",
		"assets/ekosystem/textures/dirt01.png",
		TextureParams {}
	);
	Assets::load_texture(
		"ekosystem::icon.sleep",
		"assets/ekosystem/textures/icon-sleep.png",
		TextureParams {}
	);
	Assets::load_texture(
		"ekosystem::icon.love",
		"assets/ekosystem/textures/icon-love.png",
		TextureParams {}
	);

	//	Shaders
	SharedPtr<Shader> grass_shader = Assets::load_shader(
		"ekosystem::grass",
		"assets/ekosystem/shaders/grass.vert",
		"assets/ekosystem/shaders/grass.frag"
	);

	//	Models
	SharedPtr<Model> floor_model = Assets::load_model(
		"ekosystem::floor",
		"assets/suprengine/models/cube.fbx",
		SHADER_LIT_MESH
	);
	floor_model->get_mesh( 0 )->add_texture( dirt_texture );

	Assets::load_model( "ekosystem::facing.plane", "assets/ekosystem/models/facing-plane.fbx" );

	SharedPtr<Model> wolf_model = Assets::load_model(
		"ekosystem::pawn.wolf",
		"assets/ekosystem/models/pawns/wolf.fbx",
		SHADER_LIT_MESH
	);
	wolf_model->get_mesh( 0 )->add_texture( texture );

	SharedPtr<Model> hare_model = Assets::load_model(
		"ekosystem::pawn.hare",
		"assets/ekosystem/models/pawns/hare.fbx",
		SHADER_LIT_MESH
	);
	hare_model->get_mesh( 0 )->add_texture( texture );

	SharedPtr<Model> grass_model = Assets::load_model(
		"ekosystem::pawn.grass",
		"assets/ekosystem/models/pawns/grass.fbx"
	);
	grass_model->get_mesh( 0 )->should_cull_faces = false;
	grass_model->get_mesh( 0 )->add_texture( texture );

	//	Curves
	Assets::load_curves_in_folder(
		"assets/ekosystem/curves/",
		/* is_recurive */ true,
		/* should_auto_reload */ true
	);
}

void GameInstance::init()
{
	Engine& engine = Engine::instance();
	auto inputs = engine.get_inputs();

	//  setup inputs
	inputs->set_relative_mouse_mode( false );

	//  setup render batch
	auto render_batch = get_render_batch();
	render_batch->set_debug_output( false );

	//  load scene
	engine.create_scene<GameScene>();
}

void GameInstance::release()
{}

GameInfos GameInstance::get_infos() const
{
	GameInfos infos {};
	infos.title = "EkoSystem";
	infos.width = 1280;
	infos.height = 720;
	return infos;
}
