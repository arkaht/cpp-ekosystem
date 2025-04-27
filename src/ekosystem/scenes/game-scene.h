#pragma once

#include <suprengine/core/scene.h>

#include <suprengine/components/colliders/box-collider.h>

#include <ekosystem/entities/pawn.h>
#include <ekosystem/components/camera-controller.h>
#include <ekosystem/debug-menu.h>

using namespace suprengine;

namespace suprengine
{
	class ModelRenderer;
}

namespace eks
{
	class GameScene : public Scene
	{
	public:
		GameScene();
		virtual ~GameScene();

		void init() override;

		void setup_world();

		void update( float dt ) override;

	private:
		World* _world { nullptr };
		SafePtr<CameraController> _camera_controller = nullptr;
		SafePtr<ModelRenderer> _inspector_arrow = nullptr;
		DebugMenu _debug_menu;
	};
}