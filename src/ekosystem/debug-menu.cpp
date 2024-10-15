#include "debug-menu.h"

#include <suprengine/assets.h>

#include <implot.h>

#include <array>
#include <filesystem>

using namespace eks;

DebugMenu::DebugMenu()
{
	auto& engine = Engine::instance();
	engine.on_imgui_update.listen( "eks_debug_menu",
		std::bind(
			&DebugMenu::populate,
			this
		)
	);
	engine.get_window()->on_size_changed.listen( "eks_debug_menu",
		std::bind(
			&DebugMenu::_on_window_resized,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		)
	);

	Timer timer {};
	timer.max_time = 1.0f;
	timer.repetitions = 0;
	timer.callback = std::bind( &DebugMenu::update_histogram, this );
	engine.add_timer( timer );

	_refresh_assets_ids();
}

DebugMenu::~DebugMenu()
{
	auto& engine = Engine::instance();
	engine.on_imgui_update.unlisten( "eks_debug_menu" );
}

template <typename VectorElementType, typename ElementType>
int find_index_of_element( 
	const std::vector<VectorElementType>& vector,
	const ElementType& element,
	int default_value = -1
)
{
	auto begin_itr = vector.begin();
	auto end_itr = vector.end();

	auto found_itr = std::find( begin_itr, end_itr, element );
	if ( found_itr == end_itr && default_value != -1 ) return default_value;

	return static_cast<int>( found_itr - begin_itr );
}

void DebugMenu::populate()
{
	auto& engine = Engine::instance();
	auto updater = engine.get_updater();

	//  Setup default position and size of the window
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos( { viewport->WorkPos.x + 850, viewport->WorkPos.y + 20 }, ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( { _next_window_size.x, _next_window_size.y }, ImGuiCond_FirstUseEver );

	//  Update window size and position if needed (auto-resize from application window sizes)
	if ( _should_update_window )
	{
		ImGui::SetNextWindowPos( { _next_window_pos.x, _next_window_pos.y }, ImGuiCond_None );
		ImGui::SetNextWindowSize( { _next_window_size.x, _next_window_size.y }, ImGuiCond_None );
		_should_update_window = false;
	}

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar
		| ImGuiWindowFlags_NoSavedSettings; // Disable saved settings to avoid weird sizes caused by window modes

	if ( !ImGui::Begin( "Ekosystem Debug Menu", nullptr, window_flags ) )
	{
		ImGui::End();
		return;
	}

	//  Update stored window bounds for auto-resize (user can move and resize it)
	_next_window_pos = ImGui::GetWindowPos();
	_next_window_size = ImGui::GetWindowSize();

	auto& pawns = world->get_pawns();
	auto& pawn_datas = world->get_pawn_datas();

	_refresh_pawn_datas_names( pawn_datas );

	//  Show ImGui demo window
	static bool show_demo_menu = false;
	if ( show_demo_menu ) ImGui::ShowDemoWindow( &show_demo_menu );

	//  Populate menu bar
	if ( ImGui::BeginMenuBar() )
	{
		if ( ImGui::BeginMenu( "Development" ) )
		{
			ImGui::MenuItem( "Show Demo Menu", NULL, &show_demo_menu );
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//  Populate Engine header
	if ( ImGui::CollapsingHeader( "Engine", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		auto fps = engine.get_updater()->get_fps();
		ImGui::TextColored( ImVec4 { 1.0f, 1.0f, 1.0f, 1.0f }, "FPS: %d", fps );

		//  Pause & time scale
		ImGui::Checkbox( "Pause Game", &engine.is_game_paused );
		ImGui::SliderFloat( "Time Scale", &updater->time_scale, 0.001f, 32.0f );

		//  Time scale presets
		std::array<float, 9> numbers {
			0.25f, 0.5f, 0.75f,
			1.0f,
			2.0f, 4.0f, 8.0f, 16.0f, 32.0f
		};
		for ( int i = 0; i < numbers.size(); i++ )
		{
			float scale = numbers[i];

			//  Create string buffer
			char buffer[6];
			if ( scale < 1.0f )
			{
				sprintf_s( buffer, "x%.02f", scale );
			}
			else
			{
				sprintf_s( buffer, "x%d", (int)scale );
			}

			if ( ImGui::Button( buffer ) )
			{
				updater->time_scale = scale;
			}

			if ( i < numbers.size() - 1 )
			{
				ImGui::SameLine();
			}
		}

		//  Window mode
		auto window = engine.get_window();
		const char* window_modes[3] = { "Windowed",	"Fullscreen", "Borderless Fullscreen" };
		auto current_window_mode = (int)window->get_mode();
		if ( ImGui::Combo( "Window Mode", &current_window_mode, window_modes, 3 ) )
		{
			window->set_mode( (WindowMode)current_window_mode );
		}

		ImGui::Spacing();
	}
	if ( ImGui::CollapsingHeader( "Camera", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		Camera* camera = engine.camera;

		ImGui::SeparatorText( "Projection" );

		//  Projection settings
		if ( ImGui::DragFloat( "FOV", &camera->projection_settings.fov, 1.0f, 20.0f, 120.0f ) )
		{
			camera->update_projection_from_settings();
		}
		if ( ImGui::DragFloat( "ZNear", &camera->projection_settings.znear, 1.0f, CAMERA_DEFAULT_ZNEAR, camera->projection_settings.zfar ) )
		{
			camera->update_projection_from_settings();
		}
		if ( ImGui::DragFloat( "ZFar", &camera->projection_settings.zfar, 1.0f, camera->projection_settings.znear, CAMERA_DEFAULT_ZFAR ) )
		{
			camera->update_projection_from_settings();
		}

		ImGui::SeparatorText( "Controller" );

		//  Show focused target UID or NULL
		{
			char buffer[5];
			if ( camera_controller->focus_target.is_valid() )
			{
				sprintf_s( buffer, "%04d", camera_controller->focus_target->get_owner()->get_unique_id() );
			}
			else
			{
				strcpy_s( buffer, "NULL" );
			}
			ImGui::Text( "Focused Target (UID): %s", buffer );
		}

		//  Controller settings
		ImGui::DragFloat3( "Offset", &camera_controller->camera_offset.x, 1.0f, 0.0f, 1000.0f );
		ImGui::DragFloat( "Arm Length", &camera_controller->target_arm_length, 1.0f, 1.0f, 500.0f );
		ImGui::DragFloat( "Move Speed", &camera_controller->move_speed, 1.0f, 0.0f, 500.0f );

		ImGui::Spacing();
	}
	if ( ImGui::CollapsingHeader( "Statistics", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		if ( ImPlot::BeginPlot( "Pawns Count", ImVec2 { -1.0f, 250.0f } ) )
		{
			float game_time = updater->get_accumulated_seconds();

			ImPlot::SetupAxes( "Game Time (seconds)", "Count" );
			ImPlot::SetupAxesLimits( 0, settings::HISTOGRAM_DATA_SIZE, 0, 50 );
			ImPlot::SetupAxisLimits(
				ImAxis_X1,
				math::max( 0.0f, game_time - settings::HISTOGRAM_DATA_SIZE ),
				math::max( static_cast<float>( settings::HISTOGRAM_DATA_SIZE ), game_time ),
				ImPlotCond_Always
			);
			ImPlot::SetupAxisLimitsConstraints( ImAxis_Y1, 0.0, 200.0 );

			for ( auto& pair : _pawn_histogram )
			{
				auto& name = pair.first;
				auto& histogram = pair.second;

				ImPlot::PushStyleVar( ImPlotStyleVar_FillAlpha, 0.25f );
				ImPlot::PlotShaded(
					*name,
					&histogram.data[0].x, &histogram.data[0].y,
					histogram.data.size(),
					0.0,
					ImPlotLineFlags_None,
					histogram.offset, sizeof( Vec2 )
				);
				ImPlot::PopStyleVar();

				ImPlot::PlotLine(
					*name,
					&histogram.data[0].x, &histogram.data[0].y,
					histogram.data.size(),
					ImPlotLineFlags_None,
					histogram.offset, sizeof( Vec2 )
				);
			}

			ImPlot::EndPlot();
		}

		ImGui::Spacing();
	}
	if ( ImGui::CollapsingHeader( "Ecosystem", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		_populate_pawns_table( pawns );
		_populate_selected_pawn( pawns );

		_populate_pawn_factory( pawn_datas );

		ImGui::Spacing();
	}
	if ( ImGui::CollapsingHeader( "Data Assets", ImGuiTreeNodeFlags_DefaultOpen ) )
	{
		ImGui::SeparatorText( "Pawns" );

		ImGui::PushID( "PawnEditor" );
		ImGui::Combo( "Pawn Data", &_selected_pawn_data_id,
			_pawn_datas_names.data(), (int)_pawn_datas_names.size() );

		std::string key = _pawn_datas_names[_selected_pawn_data_id];
		auto& data = pawn_datas.at( key );

		const char* create_pawn_data_popup_id = "Create Data";
		if ( ImGui::Button( "Create Data" ) )
		{
			memset( _small_input_buffer, NULL, settings::SMALL_INPUT_BUFFER_SIZE );
			ImGui::OpenPopup( create_pawn_data_popup_id );
		}

		//  Create data popup
		auto center = viewport->GetCenter();
		ImGui::SetNextWindowPos( center, ImGuiCond_Appearing, ImVec2 { 0.5f, 0.5f } );
		if ( ImGui::BeginPopupModal( create_pawn_data_popup_id, NULL, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::Text( "Choose an identifier for the new pawn data." );
			ImGui::Separator();

			ImGui::InputText( "Name", _small_input_buffer, settings::SMALL_INPUT_BUFFER_SIZE, ImGuiInputTextFlags_CharsNoBlank );

			ImGui::BeginDisabled( strlen( _small_input_buffer ) == 0 );
			if ( ImGui::Button( "Create" ) )
			{
				auto data = std::make_shared<PawnData>();
				data->name = _small_input_buffer;
				world->add_pawn_data( data );

				//  Refresh and auto-select newly created data
				_refresh_pawn_datas_names( pawn_datas, data->name );

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			if ( ImGui::Button( "Cancel" ) )
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if ( ImGui::Button( "Save Data" ) )
		{
			//  Serialize data
			json::document doc;
			data->serialize( doc );

			//  Write the JSON into a string buffer
			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );
			writer.SetMaxDecimalPlaces( 3 );  //  Fixes float values having unnecessary decimals
			doc.Accept( writer );

			//  TODO: Create and use the engine's filesystem
			//  Ensure directories are created
			std::string folder_path = "assets/ekosystem/data/pawns/";
			std::filesystem::create_directories( folder_path );

			//  Write the output into the file
			std::string file_path = folder_path + data->name + ".json";
			std::ofstream file( file_path );
			if ( file.is_open() )
			{
				file << buffer.GetString();
				file.close();
				Logger::info( "The pawn data '%s' has been saved to '%s'!", data->name.c_str(), file_path.c_str() );
			}
		}

		ImGui::Spacing();

		//  Populate pawn data editor
		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "General" ) )
		{
			//  Modulate
			float modulate[3] {
				data->modulate.r / 255.0f,
				data->modulate.g / 255.0f,
				data->modulate.b / 255.0f
			};
			if ( ImGui::ColorEdit3( "Modulate", modulate, ImGuiColorEditFlags_None ) )
			{
				data->modulate.r = (uint8_t)( modulate[0] * 255 );
				data->modulate.g = (uint8_t)( modulate[1] * 255 );
				data->modulate.b = (uint8_t)( modulate[2] * 255 );
			}
			ImGui::SetItemTooltip( "Color of the pawn" );

			//  Model name
			int model_index = find_index_of_element( _model_assets_ids, data->model_name, 0 );
			if ( 
				ImGui::Combo(
					"Model Name", &model_index,
					_model_assets_ids.data(), static_cast<int>( _model_assets_ids.size() )
				)
			)
			{
				data->model_name = _model_assets_ids[model_index];
			}

			//  Move speed
			ImGui::DragFloat( "Move Speed", &data->move_speed, 0.01f, 0.0f );
			ImGui::SetItemTooltip( "Movement speed in tile per seconds" );

			//  Curves
			int curve_index = find_index_of_element( _curve_assets_ids, data->movement_height_curve_name, 0 );
			if ( 
				ImGui::Combo(
					"Movement Height Curve Name", &curve_index,
					_curve_assets_ids.data(), static_cast<int>( _curve_assets_ids.size() )
				)
			)
			{
				data->movement_height_curve_name = _curve_assets_ids[curve_index];
			}

			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Reproduction" ) )
		{
			//  NOTE: Using pointer to 'min_child_spawn_count' 
			//  since they are stored aside.
			if ( ImGui::DragInt2( "Child Spawn Count", &data->min_child_spawn_count, 1, 0, 10 ) )
			{
				data->min_child_spawn_count = math::min( data->min_child_spawn_count, data->max_child_spawn_count );
				data->max_child_spawn_count = math::max( data->max_child_spawn_count, data->min_child_spawn_count );
			}
			ImGui::SetItemTooltip( "Amount of child to generate upon reproduction. Set to 0 to disable reproduction" );

			ImGui::DragFloat( "Min Hunger for Reproduction", &data->min_hunger_for_reproduction, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Minimum amount of hunger this pawn needs before reproducing" );

			ImGui::DragFloat( "Hunger Consumption on Reproduction", &data->hunger_consumption_on_reproduction, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Amount of hunger to consume after reproduction" );

			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Nutrition" ) )
		{
			const char* hunger_format = "%.3f hunger/s";

			ImGui::DragFloat( "Food Amount", &data->food_amount, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Amount of food this pawn provide when eaten" );

			ImGui::DragFloat( "Max Hunger", &data->max_hunger, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Maximum amount of hunger this pawn can hold" );

			ImGui::DragFloat( "Natural Hunger Consumption", &data->natural_hunger_consumption, 0.001f, 0.0f, NULL, hunger_format );
			ImGui::SetItemTooltip( "Rate of decrease of hunger per second" );

			ImGui::DragFloat( "Min Hunger to Eat", &data->min_hunger_to_eat, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Minimum amount of hunger to start eating" );

			bool has_photosynthesis = data->has_adjective( Adjectives::Photosynthesis );
			if ( !has_photosynthesis ) ImGui::BeginDisabled();
			ImGui::DragFloat( "Photosynthesis Gain", &data->photosynthesis_gain, 0.001f, 0.0f, NULL, hunger_format );
			ImGui::SetItemTooltip( "Rate of increase of hunger per second by photosynthesis" );
			if ( !has_photosynthesis ) ImGui::EndDisabled();

			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Adjectives" ) )
		{
			if ( ImGui::BeginTable( "adjectives", 3, ImGuiTableFlags_None ) )
			{
				auto adjectives = reinterpret_cast<uint32_t*>( &data->adjectives );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Photosynthesis", adjectives, (uint32_t)Adjectives::Photosynthesis );
				ImGui::SetItemTooltip( "Consume light as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Carnivore", adjectives, (uint32_t)Adjectives::Carnivore );
				ImGui::SetItemTooltip( "Consume Meat as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Herbivore", adjectives, (uint32_t)Adjectives::Herbivore );
				ImGui::SetItemTooltip( "Consume Vegetal as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Meat", adjectives, (uint32_t)Adjectives::Meat );
				ImGui::SetItemTooltip( "Is eatable by Carnivore" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Vegetal", adjectives, (uint32_t)Adjectives::Vegetal );
				ImGui::SetItemTooltip( "Is eatable by Herbivore" );

				ImGui::EndTable();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
		ImGui::Spacing();
	}

	ImGui::End();
}

void DebugMenu::update_histogram()
{
	//	Create pawn counters
	std::unordered_map<std::string, int> pawn_counters {};
	pawn_counters.reserve( _pawn_datas_names.size() );
	for ( const char* name : _pawn_datas_names )
	{
		pawn_counters.emplace( name, 0 );
	}

	//	Count number of pawns per data
	auto& pawns = world->get_pawns();
	for ( auto& pawn : pawns )
	{
		auto& key = pawn->data->name;
		auto itr = pawn_counters.find( key );
		if ( itr == pawn_counters.end() ) continue;

		itr->second++;
	}

	float game_time = Engine::instance().get_updater()->get_accumulated_seconds();
	for ( auto& pair : pawn_counters )
	{
		auto& key = pair.first;
		auto& counter = pair.second;

		//	Create histogram for pawn's data
		auto itr = _pawn_histogram.find( key );
		if ( itr == _pawn_histogram.end() )
		{
			_pawn_histogram.emplace( key, ScrollingBuffer( settings::HISTOGRAM_DATA_SIZE ) );
			itr = _pawn_histogram.find( key );
		}

		//	Insert counter
		itr->second.add_point( Vec2 { game_time, static_cast<float>( counter ) } );
	}
}

void DebugMenu::_refresh_assets_ids()
{
	_model_assets_ids = Assets::get_assets_as_ids<Model, const char*>();
	_model_assets_ids.emplace( _model_assets_ids.begin(), "none" );

	_curve_assets_ids = Assets::get_assets_as_ids<Curve, const char*>();
	_curve_assets_ids.emplace( _curve_assets_ids.begin(), "none" );
}

void DebugMenu::_refresh_pawn_datas_names(
	const std::map<std::string, SharedPtr<PawnData>>& pawn_datas,
	const std::string& auto_select_name
)
{
	//  Construct items list for combo
	int i = 0;
	_pawn_datas_names.clear();
	_pawn_datas_names.resize( pawn_datas.size() );
	for ( auto& pair : pawn_datas )
	{
		_pawn_datas_names[i] = pair.second->name.c_str();

		//  Auto-select
		if ( !auto_select_name.empty() && auto_select_name == pair.second->name )
		{
			_selected_pawn_data_id = i;
		}

		i++;
	}
}

void DebugMenu::_populate_pawns_table(
	const std::vector<SafePtr<Pawn>>& pawns
)
{
	ImGuiTableFlags table_flags =
		ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders |
		ImGuiTableFlags_RowBg;

	//  Populate pawns
	ImGui::SeparatorText( "Pawns" );
	if ( ImGui::BeginTable( "eks_pawns", 4, table_flags, { 0.0f, 150.0f } ) )
	{
		ImGui::TableSetupColumn( "UID", ImGuiTableColumnFlags_WidthFixed );
		ImGui::TableSetupColumn( "Name", ImGuiTableColumnFlags_WidthFixed );
		ImGui::TableSetupColumn( "Action", ImGuiTableColumnFlags_WidthFixed );
		ImGui::TableSetupColumn( "Hunger", ImGuiTableColumnFlags_WidthFixed );
		ImGui::TableSetupScrollFreeze( 0, 1 );
		ImGui::TableHeadersRow();

		ImGuiSelectableFlags selectable_flags =
			ImGuiSelectableFlags_SpanAllColumns
			| ImGuiSelectableFlags_AllowOverlap;
		for ( int i = 0; i < pawns.size(); i++ )
		{
			ImGui::PushID( i );
			ImGui::TableNextRow( ImGuiTableRowFlags_None );

			auto& pawn = pawns[i];
			if ( pawn.is_valid() )
			{
				//  Column 0: ID
				ImGui::TableSetColumnIndex( 0 );
				char buffer[5];
				sprintf_s( buffer, "%04d", pawn->get_unique_id() );
				if ( ImGui::Selectable(
					buffer,
					_selected_pawn_id == i,
					selectable_flags
				) )
				{
					_selected_pawn_id = i;
				}

				//  Column 1: Name
				ImGui::TableSetColumnIndex( 1 );
				ImGui::Text( pawn->get_name().c_str() );

				//  Column 2: Action
				ImGui::TableSetColumnIndex( 2 );
				if ( ImGui::SmallButton( "Focus" ) )
				{
					camera_controller->focus_target =
						pawn->transform;
				}
				ImGui::SameLine();
				if ( ImGui::SmallButton( "Kill" ) )
				{
					pawn->kill();
				}

				//  Column 3: Hunger
				ImGui::TableSetColumnIndex( 3 );
				float hunger_ratio = math::clamp(
					pawn->hunger / pawn->data->max_hunger,
					0.0f, 1.0f
				);
				ImGui::Extra::ColoredProgressBar(
					hunger_ratio,
					settings::HUNGER_COLOR,
					{ 0.0f, 0.0f }
				);
			}
			else
			{
				ImGui::TableSetColumnIndex( 0 );
				ImGui::Selectable( "????", _selected_pawn_id == i, selectable_flags );

				ImGui::TableSetColumnIndex( 1 );
				ImGui::TextUnformatted( "N/A" );

				ImGui::TableSetColumnIndex( 2 );
				ImGui::TextUnformatted( "N/A" );

				ImGui::TableSetColumnIndex( 3 );
				ImGui::TextUnformatted( "0%" );
			}

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	if ( ImGui::Button( "Purge" ) )
	{
		for ( auto& pawn : pawns )
		{
			pawn->kill();
		}
	}
	ImGui::SetItemTooltip( "Kill all existing pawns" );

	ImGui::Spacing();
}

void DebugMenu::_populate_pawn_factory(
	const std::map<std::string, SharedPtr<PawnData>>& pawn_datas
)
{
	ImGui::SeparatorText( "Pawn Factory" );

	ImGui::Combo( "Pawn Data##Factory", &_selected_pawn_data_id,
		_pawn_datas_names.data(), (int)_pawn_datas_names.size() );

	if ( ImGui::Button( "Create Pawn" ) )
	{
		std::string key = _pawn_datas_names[_selected_pawn_data_id];
		auto& data = pawn_datas.at( key );
		for ( int i = 0; i < _spawn_count; i++ )
		{
			Vec3 pos = world->find_random_tile_pos();
			auto pawn = world->create_pawn( data, pos );
			pawn->hunger = _hunger_ratio * data->max_hunger;
			pawn->group_id = _group_id;
		}
	}

	//  Spawn count
	ImGui::SameLine();
	ImGui::SetNextItemWidth( 70.0f );
	if ( ImGui::InputInt( "Spawn Count", &_spawn_count, 1, 10 ) )
	{
		_spawn_count = math::clamp( _spawn_count, 1, 99 );
	}

	//  Hunger ratio
	ImGui::Extra::DragPercent( "Hunger Ratio", &_hunger_ratio, 0.01f, 0.0f, 1.0f );
	ImGui::SetItemTooltip( "Start hunger ratio (computed using max hunger) for new pawns" );

	//  Group ID
	int group_id = _group_id;
	if ( ImGui::InputInt( "Group ID", &group_id, 1, 10 ) )
	{
		_group_id = (GroupID)math::clamp( group_id, 0, (int)std::numeric_limits<GroupID>::max() );
	}
	ImGui::SetItemTooltip(
		"Group identifier for new pawns inherited to childs and preventing them from eating each other.\n"
		"Set to 0 for no groups"
	);

	ImGui::Spacing();
}

void DebugMenu::_populate_selected_pawn( const std::vector<SafePtr<Pawn>>& pawns )
{
	if ( !ImGui::TreeNode( "Selected Pawn" ) ) return;

	if ( _selected_pawn_id < 0 || _selected_pawn_id >= pawns.size() )
	{
		ImGui::Text( "Select a pawn first to inspect it." );
		ImGui::TreePop();
		return;
	}

	auto& pawn = pawns[_selected_pawn_id];
	if ( pawn.is_valid() )
	{
		ImGui::Text( ( "Name: " + pawn->get_name() ).c_str() );

		//  Compute hunger
		float hunger = pawn->hunger;
		float max_hunger = pawn->data->max_hunger;
		float hunger_ratio = math::clamp(
			hunger / max_hunger,
			0.0f,
			1.0f
		);

		//  Create progress text
		char buffer[32];
		sprintf_s(
			buffer, "%.03f/%.03f (%d%%)",
			hunger, max_hunger, (int)( hunger_ratio * 100 )
		);

		//  Display hunger
		ImGui::Extra::ColoredProgressBar(
			hunger_ratio,
			settings::HUNGER_COLOR,
			{ 0.0f, 0.0f },
			buffer
		);
		ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );
		ImGui::Text( "Hunger" );
	}

	ImGui::TreePop();
}

void DebugMenu::_on_window_resized( const Vec2& new_size, const Vec2& old_size )
{
	float ratio = new_size.x / old_size.x;
	_next_window_size *= ratio;
	_next_window_pos *= ratio;
	_should_update_window = true;
}
