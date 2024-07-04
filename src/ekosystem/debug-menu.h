#pragma once

#include <suprengine/engine.h>

#include "world.h"
#include "entities/pawn.h"
#include "components/camera-controller.h"
#include "data/pawn-data.hpp"

namespace eks
{
	using namespace suprengine;

	namespace settings
	{
		constexpr ImVec4 HUNGER_COLOR { 0.8f, 0.3f, 0.1f, 1.0f };
	}

	class DebugMenu
	{
	public:
		DebugMenu();
		~DebugMenu();

		void populate();


	public:
		World* world;
		SafePtr<CameraController> camera_controller;

	private:
		void _populate_pawns_table( 
			const std::vector<SafePtr<Pawn>>& pawns
		);
		void _populate_pawn_factory(
			const std::map<std::string, SharedPtr<PawnData>> pawn_datas
		);
		void _populate_selected_pawn(
			const std::vector<SafePtr<Pawn>>& pawns 
		);

	private:
		int _selected_pawn_id = 0;
		int _selected_pawn_data_id = 0;

		std::vector<const char*> _pawn_datas_names;
	};
}