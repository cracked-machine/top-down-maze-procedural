#include <Components/Persistent/ArmedOffDelay.hpp>
#include <Components/Persistent/CameraSmoothSpeed.hpp>
#include <Components/Persistent/CryptNpcSpawnCount.hpp>
#include <Components/Persistent/NpcShockwaveFreq.hpp>
#include <Components/Persistent/NpcShockwaveMaxRadius.hpp>
#include <Components/Persistent/NpcShockwaveResolution.hpp>
#include <Components/Persistent/NpcShockwaveSpeed.hpp>
#include <Components/Persistent/PlayerLerpInterruptThreshold.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>

#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>

#include "imgui-SFML.h"
#include <imgui.h>

#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Components/Player/PlayerKeysCount.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Threats/BombSystem.hpp>
#include <Utils/Player.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys
{

void RenderMenuSystem::init_title()
{
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );
  m_title_screen_shader = std::make_unique<Sprites::TitleScreenShader>( "res/shaders/TitleScreen.frag", display_size );
  m_title_screen_shader->setup();
}

void RenderMenuSystem::render_title()
{
  // main render begin
  m_window.clear();
  {
    // shaders
    m_title_screen_shader->set_position( { 0, 0 } );
    const auto mouse_pos = sf::Vector2f( sf::Mouse::getPosition( m_window ) ).componentWiseDiv( sf::Vector2f( m_window.getSize() ) );
    m_title_screen_shader->update( mouse_pos );
    m_window.draw( *m_title_screen_shader );

    // text
    sf::Color txt_color{ 64, 96, 184 };
    sf::Color txt_outline_color{ 48, 48, 48 };
    sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );

    render_text( "Nekropolis", display_size.x / 20, { display_size.x * 0.25f, display_size.y * 0.25f }, Alignment::CENTER, 2.f, txt_outline_color );

    render_text( "Press <Enter> key to start", display_size.x / 40, { display_size.x * 0.25f, display_size.y * 0.5f }, Alignment::CENTER, 3.f,
                 txt_color );

    render_text( "Press <Q> key to quit", display_size.x / 40, { display_size.x * 0.25f, display_size.y * 0.5f + 100.f }, Alignment::CENTER, 3.f,
                 txt_color );

    render_text( "Press <S> key for settings", display_size.x / 40, { display_size.x * 0.25f, display_size.y * 0.5f + 200.f }, Alignment::CENTER, 3.f,
                 txt_color );
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_settings( sf::Time globalDeltaTime )
{
  m_window.clear();
  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );
  sf::Text title_text( m_font, "Settings", display_size.x / 20 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { 10.f, 10.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press <Esc> key to go back", display_size.x / 40 );
  start_text.setFillColor( sf::Color::White );
  auto right_align_px = display_size.x - start_text.getGlobalBounds().size.x - 10.f;
  start_text.setPosition( { right_align_px, 50.f } );
  m_window.draw( start_text );

  // ImGUI should be rendered before window.display() or SFML wipes the display buffer prematurely
  render_settings_widgets( globalDeltaTime, title_text.getLocalBounds() );

  m_window.display();
}

const std::vector<sf::Vector2u> RenderMenuSystem::DisplaySettings::resolutions = { { 1920, 1080 }, { 1680, 1050 }, { 1600, 900 }, { 1440, 900 },
                                                                                   { 1366, 768 },  { 1280, 720 },  { 1024, 768 }, { 800, 600 } };

void RenderMenuSystem::render_settings_widgets( sf::Time globalDeltaTime, sf::FloatRect title_bounds )
{
  auto padding_px = 10;
  // need to make sure we call Update() and Render() every frame
  ImGui::SFML::Update( m_window, globalDeltaTime );
  ImGui::Begin( "Settings", nullptr, kImGuiWindowOptions );
  try
  {
    // keep imgui window in sync with display resolution updates
    auto &display_resolution = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );
    const float kFontScaleFactor = 1280.f;

    ImVec2 window_size = ImVec2( display_resolution.x - padding_px, display_resolution.y - ( title_bounds.size.y * 2 ) - padding_px );
    ImGui::SetWindowSize( "Settings", window_size );
    ImGui::SetWindowPos( "Settings", ImVec2( padding_px, title_bounds.size.y * 2 ) );
    ImGui::SetWindowFontScale( display_resolution.x / kFontScaleFactor );

    static int current_resolution = 0; // Default to first item

    // Capture resolutions by reference in the lambda
    if ( ImGui::Combo( "Display Resolution", &current_resolution, DisplaySettings::get, nullptr, DisplaySettings::resolutions.size() ) )
    {
      // Resolution changed - apply the selected resolution
      auto &display_resolution = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );
      display_resolution = DisplaySettings::resolutions[current_resolution];

      m_window.create( sf::VideoMode( display_resolution ), "Your Game Title", sf::State::Fullscreen );
      m_window.setVerticalSyncEnabled( true );
      m_title_screen_shader->resize_texture( display_resolution );
      init_title();

      SPDLOG_DEBUG( "Selected resolution: {}x{}", display_resolution.x, display_resolution.y );
    }

    ImGui::SeparatorText( "Player Settings" );

    Sys::PersistSystem::get<Cmp::Persist::BlastRadius>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerAnimFramerate>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerLerpSpeed>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerDiagonalLerpSpeedModifier>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerFootstepAddDelay>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerFootstepFadeDelay>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerShortcutLerpSpeedModifier>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::DiggingCooldownThreshold>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::DiggingDamagePerHit>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::WeaponDegradePerHit>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerDetectionScale>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PcDamageDelay>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::CameraSmoothSpeed>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::PlayerLerpInterruptThreshold>( getReg() ).render_widget();

    // Bomb Settings
    ImGui::SeparatorText( "Bomb Settings" );

    Sys::PersistSystem::get<Cmp::Persist::BombDamage>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::FuseDelay>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::ArmedOnDelay>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::ArmedOffDelay>( getReg() ).render_widget();

    // Hazard Settings
    ImGui::SeparatorText( "Hazard Settings" );

    Sys::PersistSystem::get<Cmp::Persist::CorruptionDamage>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::WormholeAnimFramerate>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::WormholeSeed>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::CorruptionSeed>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::SinkholeSeed>( getReg() ).render_widget();

    // Loot Settings
    ImGui::SeparatorText( "Loot Settings" );

    Sys::PersistSystem::get<Cmp::Persist::HealthBonus>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::BombBonus>( getReg() ).render_widget();

    // NPC Settings
    ImGui::SeparatorText( "NPC Settings" );

    Sys::PersistSystem::get<Cmp::Persist::NpcSkeleAnimFramerate>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcGhostAnimFramerate>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcDamage>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::CryptNpcSpawnCount>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveResolution>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveSpeed>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveFreq>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcShockwaveMaxRadius>( getReg() ).render_widget();

    auto &npc_push_back = Sys::PersistSystem::get<Cmp::Persist::NpcPushBack>( getReg() );
    if ( ImGui::SliderScalar( "NPC Push Back Distance", ImGuiDataType_Float, npc_push_back.get_value_ptr(), npc_push_back.get_min_value_ptr(),
                              npc_push_back.get_max_value_ptr(), "%.1f pixels" ) )
    {
      float step = Constants::kGridSizePxF.x;
      npc_push_back.get_value() = roundf( npc_push_back.get_value() / step ) * step;
    }

    Sys::PersistSystem::get<Cmp::Persist::NpcDeathAnimFramerate>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcActivateScale>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcScanScale>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::NpcLerpSpeed>( getReg() ).render_widget();

    // Audio
    ImGui::SeparatorText( "Audio" );

    Sys::PersistSystem::get<Cmp::Persist::MusicVolume>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::EffectsVolume>( getReg() ).render_widget();

    // Procedural Generation
    ImGui::SeparatorText( "Procedural Generation" );

    Sys::PersistSystem::get<Cmp::Persist::MaxNumAltars>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::GraveNumMultiplier>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::ExitKeyRequirement>( getReg() ).render_widget();
    Sys::PersistSystem::get<Cmp::Persist::MaxNumCrypts>( getReg() ).render_widget();

  } catch ( const std::exception &e )
  {
    ImGui::TextColored( ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ), "Error rendering settings: %s", e.what() );
  }
  ImGui::End();
  ImGui::SFML::Render( m_window );
}

void RenderMenuSystem::render_paused( sf::Time globalDeltaTime )
{
  // main render begin
  m_window.clear();

  ImGui::SFML::Update( m_window, globalDeltaTime );
  ImGui::Begin( "PausedMenu", nullptr, kImGuiWindowOptions );

  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );

  sf::Text title_text( m_font, "Paused", 96 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { display_size.x / 4.f, 100.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press P to continue", 48 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { display_size.x / 4.f, 200.f } );
  m_window.draw( start_text );

  auto &music_volume = Sys::PersistSystem::get<Cmp::Persist::MusicVolume>( getReg() );
  ImGui::SliderFloat( "Music Volume", &music_volume.get_value(), 0.f, 100.f, "%.1f" );

  auto &effects_volume = Sys::PersistSystem::get<Cmp::Persist::EffectsVolume>( getReg() );
  ImGui::SliderFloat( "Effects Volume", &effects_volume.get_value(), 0.f, 100.f, "%.1f" );

  ImGui::End();
  ImGui::SFML::Render( m_window );

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_defeat_screen()
{
  // main render begin
  m_window.clear();
  {

    sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );

    sf::Text title_text( m_font, "You died...again!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { display_size.x / 4.f, 100.f } );
    m_window.draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { display_size.x / 4.f, 200.f } );
    m_window.draw( start_text );
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_victory_screen()
{
  // main render begin
  m_window.clear();

  sf::Vector2u display_size = Sys::PersistSystem::get<Cmp::Persist::DisplayResolution>( getReg() );

  sf::Text title_text( m_font, "You won!", 96 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { display_size.x / 4.f, 100.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press <R> key to continue", 48 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { display_size.x / 4.f, 200.f } );
  m_window.draw( start_text );

  auto [inventory_entt, inventory_type] = Utils::Player::get_player_inventory_type( getReg() );
  sf::Text player_inventory_text( m_font, "Player Inventory: " + ( inventory_type = "" ? "None" : inventory_type ), 24 );
  player_inventory_text.setFillColor( sf::Color::White );
  player_inventory_text.setPosition( { display_size.x / 4.f, 550.f } );
  m_window.draw( player_inventory_text );

  m_window.display();
  // main render end
}

} // namespace ProceduralMaze::Sys