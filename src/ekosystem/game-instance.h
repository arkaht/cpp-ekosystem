#pragma once

#include <suprengine/core/game.h>

#include <suprengine/rendering/opengl/opengl-render-batch.h>

namespace eks
{
	using namespace suprengine;

	class GameInstance : public suprengine::Game<OpenGLRenderBatch>
	{
		void load_assets() override;

		void init() override;
		void release() override;

		GameInfos get_infos() const override;
	};
}