#include "game-instance.h"

#include <suprengine/assets.h>

#include <ekosystem/scenes/game-scene.h>

using namespace eks;

void GameInstance::load_assets()
{
	SharedPtr<Texture> texture = Assets::get_texture( TEXTURE_MEDIUM_GRID );

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
	render_batch->set_background_color( Color::from_0x( 0x8CDEDCFF ) );

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
