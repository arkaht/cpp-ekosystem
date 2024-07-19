#include "debug-menu.h"

#include <array>

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
}

DebugMenu::~DebugMenu()
{
	auto& engine = Engine::instance();
	engine.on_imgui_update.unlisten( "eks_debug_menu" );
}

void DebugMenu::populate()
{
	auto& engine = Engine::instance();
	auto updater = engine.get_updater();

	//  Setup default position and size of the window
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos( { viewport->WorkPos.x + 850, viewport->WorkPos.y + 20 }, ImGuiCond_FirstUseEver );
	ImGui::SetNextWindowSize( { 400, 680 }, ImGuiCond_FirstUseEver );

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;

	if ( !ImGui::Begin( "Ekosystem Debug Menu", nullptr, window_flags ) )
	{
		ImGui::End();
		return;
	}

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

		if ( ImGui::Button( "Create Data" ) )
		{
			//world->
		}

		ImGui::Spacing();

		//  Populate pawn data editor
		std::string key = _pawn_datas_names[_selected_pawn_data_id];
		auto& data = pawn_datas.at( key );

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

			//  Move speed
			ImGui::DragFloat( "Move Speed", &data->move_speed, 0.01f, 0.0f );
			ImGui::SetItemTooltip( "Movement speed in tile per seconds" );

			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Reproduction" ) )
		{
			ImGui::InputInt( "Child Spawn Count", &data->child_spawn_count );
			ImGui::SetItemTooltip( "Amount of child to generate upon reproduction. Set to 0 to disable reproduction" );
			
			ImGui::DragFloat( "Min Food Reproduction", &data->min_food_reproduction, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Minimum amount of hunger this pawn needs before reproducing" );
			
			ImGui::DragFloat( "Food Loss on Reproduction", &data->food_loss_on_reproduction, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Amount of food to lose after reproduction" );
			
			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Nutrition" ) )
		{
			ImGui::DragFloat( "Food Amount", &data->food_amount, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Amount of food this pawn provide when eaten" );
			
			ImGui::DragFloat( "Max Hunger", &data->max_hunger, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Maximum amount of hunger this pawn can hold" );
			
			ImGui::DragFloat( "Hunger Gain Rate", &data->hunger_gain_rate, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Rate of increase of hunger per second" );
			
			ImGui::DragFloat( "Min Hunger to Eat", &data->min_hunger_to_eat, 0.001f, 0.0f );
			ImGui::SetItemTooltip( "Minimum amount of hunger to start eating" );
			
			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "Adjectives" ) )
		{
			if ( ImGui::BeginTable( "adjectives", 3, ImGuiTableFlags_None ) )
			{
				auto adjectives = reinterpret_cast<uint32_t*>( &data->adjectives );
			
				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Photosynthesis",	adjectives, (uint32_t)Adjectives::Photosynthesis );
				ImGui::SetItemTooltip( "Consume light as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Carnivore",		adjectives, (uint32_t)Adjectives::Carnivore );
				ImGui::SetItemTooltip( "Consume Meat as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Herbivore",		adjectives, (uint32_t)Adjectives::Herbivore );
				ImGui::SetItemTooltip( "Consume Vegetal as food" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Meat",			adjectives, (uint32_t)Adjectives::Meat );
				ImGui::SetItemTooltip( "Is eatable by Carnivore" );

				ImGui::TableNextColumn();
				ImGui::CheckboxFlags( "Vegetal",		adjectives, (uint32_t)Adjectives::Vegetal );
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

void DebugMenu::_refresh_pawn_datas_names( 
	const std::map<std::string, SharedPtr<PawnData>>& pawn_datas
)
{
	//  Construct items list for combo
	int i = 0;
	_pawn_datas_names.clear();
	_pawn_datas_names.resize( pawn_datas.size() );
	for ( auto& pair : pawn_datas )
	{
		_pawn_datas_names[i] = pair.second->name.c_str();
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
		world->create_pawn( data, world->find_random_tile_pos() );
	}

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
