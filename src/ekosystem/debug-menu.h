#pragma once

#include <suprengine/engine.h>

#include "world.h"
#include "entities/pawn.h"
#include "components/camera-controller.h"
#include "data/pawn-data.h"

namespace eks
{
	using namespace suprengine;

	namespace settings
	{
		constexpr ImVec4 HUNGER_COLOR { 0.8f, 0.3f, 0.1f, 1.0f };
		constexpr size_t SMALL_INPUT_BUFFER_SIZE = 32;
	}

	class DebugMenu
	{
	public:
		DebugMenu();
		~DebugMenu();

		void populate();

	public:
		World* world { nullptr };
		SafePtr<CameraController> camera_controller;

	private:
		void _refresh_assets_ids();
		void _refresh_pawn_datas_names(
			const std::map<std::string, SharedPtr<PawnData>>& pawn_datas,
			const std::string& auto_select_name = ""
		);

		void _populate_pawns_table(
			const std::vector<SafePtr<Pawn>>& pawns
		);
		void _populate_pawn_factory(
			const std::map<std::string, SharedPtr<PawnData>>& pawn_datas
		);
		void _populate_selected_pawn(
			const std::vector<SafePtr<Pawn>>& pawns
		);

		void _on_window_resized( const Vec2& new_size, const Vec2& old_size );

	private:
		int _selected_pawn_id = 0;
		int _selected_pawn_data_id = 0;

		int _spawn_count = 1;
		float _hunger_ratio = 1.0f;
		GroupID _group_id = 0;

		char _small_input_buffer[settings::SMALL_INPUT_BUFFER_SIZE] = "";

		Vec2 _next_window_size { 400.0f, 680.0f };
		Vec2 _next_window_pos { 0.0f, 0.0f };
		bool _should_update_window = false;

		std::vector<const char*> _model_assets_ids {};
		std::vector<const char*> _curve_assets_ids {};

		std::vector<const char*> _pawn_datas_names {};
	};
}