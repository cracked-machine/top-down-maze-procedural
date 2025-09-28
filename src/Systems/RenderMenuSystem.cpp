#include <BombSystem.hpp>
#include <Persistent/ArmedOnDelay.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/BombDamage.hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/NPCActivateScale.hpp>
#include <Persistent/NPCScanScale.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Persistent/PlayerLerpSpeed.hpp>
#include <Persistent/PlayerMinVelocity.hpp>
#include <Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Persistent/PlayerSubmergedlLerpSpeedModifier.hpp>
#include <Persistent/WaterBonus.hpp>
#include <SFML/System/Angle.hpp>
#include <Systems/RenderMenuSystem.hpp>
#include <imgui.h>

namespace ProceduralMaze::Sys {

void RenderMenuSystem::render_loading_screen( const std::string &status )
{
  getWindow().clear();

  // Background
  sf::RectangleShape background( ( sf::Vector2f( kDisplaySize ) ) );
  background.setFillColor( sf::Color::Black );
  getWindow().draw( background );

  // Loading text
  render_text( "Loading Game...", 64, { kDisplaySize.x * 0.5f, kDisplaySize.y * 0.4f }, Alignment::CENTER );
  render_text( status, 32, { kDisplaySize.x * 0.5f, kDisplaySize.y * 0.6f }, Alignment::CENTER );

  // Simple progress bar or spinner
  static float rotation = 0.0f;
  rotation += 5.0f;
  sf::CircleShape spinner( 20.0f, 8 );
  spinner.setPosition( { kDisplaySize.x * 0.5f - 20.0f, kDisplaySize.y * 0.7f } );
  spinner.setFillColor( sf::Color::White );
  spinner.setRotation( sf::degrees( rotation ) );
  getWindow().draw( spinner );

  getWindow().display();
}

void RenderMenuSystem::render_title()
{
  // main render begin
  getWindow().clear();
  {
    m_title_screen_shader.set_position( { 0, 0 } );
    const auto mouse_pos = sf::Vector2f( sf::Mouse::getPosition( getWindow() ) )
                               .componentWiseDiv( sf::Vector2f( getWindow().getSize() ) );
    m_title_screen_shader.update( mouse_pos );
    getWindow().draw( m_title_screen_shader );

    render_text( "Procedural Maze", 128, { kDisplaySize.x * 0.25f, 100.f }, Alignment::CENTER, 20.f, sf::Color::Black,
                 sf::Color::White );

    render_text( "Press <Enter> key to start", 48, { kDisplaySize.x * 0.25f, 300.f }, Alignment::CENTER, 15.f );

    render_text( "Press <Q> key to quit", 48, { kDisplaySize.x * 0.25f, 400.f }, Alignment::CENTER, 15.f );

    render_text( "Press <S> key for settings", 48, { kDisplaySize.x * 0.25f, 500.f }, Alignment::CENTER, 15.f );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_settings( sf::Time deltaTime )
{
  getWindow().clear();

  sf::Text title_text( m_font, "Settings", 64 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { 100.f, 50.f } );
  getWindow().draw( title_text );

  sf::Text start_text( m_font, "Press <Esc> key to go back", 24 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { 400.f, 95.f } );
  getWindow().draw( start_text );

  // ImGUI should be rendered before window.display() or SFML wipes the display buffer prematurely
  render_settings_widgets( deltaTime );

  getWindow().display();
}

void RenderMenuSystem::render_settings_widgets( sf::Time deltaTime )
{
  // need to make sure we call Update() and Render() every frame
  ImGui::SFML::Update( getWindow(), deltaTime );

  ImGui::Begin( "Settings", nullptr, kImGuiWindowOptions );
  // See PlayerSystem::Settings for details of which Components these are set

  auto &bomb_inventory = m_reg->ctx().get<Cmp::Persistent::BombInventory>();
  if ( ImGui::InputInt( "Bomb Inventory", &bomb_inventory() ) )
  {
    bomb_inventory() = std::clamp( bomb_inventory(), -1, 100 );
  }

  auto &blast_radius = m_reg->ctx().get<Cmp::Persistent::BlastRadius>();
  ImGui::SliderInt( "Blast Radius", &blast_radius(), 1, 3 );

  auto &player_lerp_speed = m_reg->ctx().get<Cmp::Persistent::PlayerLerpSpeed>();
  ImGui::SliderFloat( "Player Lerp Speed", &player_lerp_speed(), 3.f, 10.f, "%.1f" );

  auto &player_diagonal_lerp_speed_modifier = m_reg->ctx().get<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Diagonal Lerp Speed Modifier", &player_diagonal_lerp_speed_modifier(), 0.001f, 1.f,
                      "%.2f" );

  auto &player_shortcut_lerp_speed_modifier = m_reg->ctx().get<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Shortcut Lerp Speed Modifier", &player_shortcut_lerp_speed_modifier(), 0.001f, 1.f,
                      "%.2f" );

  auto &player_submerged_lerp_speed_modifier = m_reg->ctx().get<Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Submerged Lerp Speed Modifier", &player_submerged_lerp_speed_modifier(), 0.001f, 1.f,
                      "%.2f" );

  auto &land_max_speed = m_reg->ctx().get<Cmp::Persistent::LandMaxSpeed>();
  ImGui::SliderFloat( "Land Max Speed", &land_max_speed(), 25.0f, 150.0f, "%.1f" );
  auto &water_max_speed = m_reg->ctx().get<Cmp::Persistent::WaterMaxSpeed>();
  ImGui::SliderFloat( "Water Max Speed", &water_max_speed(), 25.0f, 150.0f, "%.1f" );

  auto &player_min_velocity = m_reg->ctx().get<Cmp::Persistent::PlayerMinVelocity>();
  ImGui::SliderFloat( "Player Min Velocity", &player_min_velocity(), 0.f, 5.f, "%.1f" );

  auto &land_acceleration = m_reg->ctx().get<Cmp::Persistent::LandAcceleration>();
  ImGui::SliderFloat( "Above Water Acceleration Rate", &land_acceleration(), 100.f, 1000.f, "%.1f pixels/second²" );
  auto &land_deceleration = m_reg->ctx().get<Cmp::Persistent::LandDeceleration>();
  ImGui::SliderFloat( "Above Water Deceleration Rate", &land_deceleration(), 100.f, 1000.f, "%.1f pixels/second²" );
  auto &water_acceleration = m_reg->ctx().get<Cmp::Persistent::WaterAcceleration>();
  ImGui::SliderFloat( "Under Water Acceleration Rate", &water_acceleration(), 50.f, 500.f, "%.1f pixels/second²" );
  auto &water_deceleration = m_reg->ctx().get<Cmp::Persistent::WaterDeceleration>();
  ImGui::SliderFloat( "Under Water Deceleration Rate", &water_deceleration(), 50.f, 500.f, "%.1f pixels/second²" );
  ImGui::Separator();
  auto &flood_speed = m_reg->ctx().get<Cmp::Persistent::FloodSpeed>();
  ImGui::SliderFloat( "Flood Velocity", &flood_speed(), 1.f, 10.f, "%.1f pixels/second" );
  ImGui::Separator();

  auto &npc_damage = m_reg->ctx().get<Cmp::Persistent::NpcDamage>();
  ImGui::SliderInt( "NPC Damage", &npc_damage(), 1, 50 );

  auto &bomb_damage = m_reg->ctx().get<Cmp::Persistent::BombDamage>();
  ImGui::SliderInt( "Bomb Damage", &bomb_damage(), 1, 50 );

  auto &fuse_delay = m_reg->ctx().get<Cmp::Persistent::FuseDelay>();
  ImGui::SliderFloat( "Fuse Delay", &fuse_delay(), 1.f, 10.f, "%.1f seconds" );

  auto &armed_on_delay = m_reg->ctx().get<Cmp::Persistent::ArmedOnDelay>();
  if ( ImGui::InputFloat( "Armed Detonation Delay Increment", &armed_on_delay(), 0.001f, 0.001f ) )
  {
    armed_on_delay() = std::clamp( armed_on_delay(), 0.001f, 0.5f );
  }
  auto &armed_off_delay = m_reg->ctx().get<Cmp::Persistent::ArmedOffDelay>();
  if ( ImGui::InputFloat( "Armed Off Delay", &armed_off_delay(), 0.001f, 0.001f ) )
  {
    armed_off_delay() = std::clamp( armed_off_delay(), 0.001f, 0.5f );
  }

  ImGui::Separator();

  auto &health_bonus = m_reg->ctx().get<Cmp::Persistent::HealthBonus>();
  ImGui::SliderInt( "Health Bonus", &health_bonus(), 1, 50 );
  auto &bomb_bonus = m_reg->ctx().get<Cmp::Persistent::BombBonus>();
  ImGui::SliderInt( "Bomb Bonus", &bomb_bonus(), 1, 20 );
  auto &water_bonus = m_reg->ctx().get<Cmp::Persistent::WaterBonus>();
  ImGui::SliderFloat( "Water Bonus", &water_bonus(), 10.f, 500.f, "%.1f units" );

  auto &obstacle_push_back = m_reg->ctx().get<Cmp::Persistent::ObstaclePushBack>();
  if ( ImGui::InputFloat( "Obstacle Push Back Factor", &obstacle_push_back(), 0.1f, 0.1f, "%.2f" ) )
  {
    obstacle_push_back() = std::clamp( obstacle_push_back(), 1.0f, 5.0f );
  }
  auto &npc_push_back = m_reg->ctx().get<Cmp::Persistent::NpcPushBack>();
  if ( ImGui::InputFloat( "NPC Push Back Distance", &npc_push_back(), 0.1f, 0.1f, "%.2f pixels" ) )
  {
    npc_push_back() = std::clamp( npc_push_back(), 1.0f, 50.0f );
  }

  auto &npc_activate_scale = m_reg->ctx().get<Cmp::Persistent::NPCActivateScale>();
  ImGui::SliderFloat( "NPC Activation Bounding Box Scale Factor", &npc_activate_scale(), 1.f, 20.f, "%.1f pixels" );
  auto &npc_scan_scale = m_reg->ctx().get<Cmp::Persistent::NPCScanScale>();
  ImGui::SliderFloat( "NPC Scan Bounding Box Scale Factor", &npc_scan_scale(), 1.f, 3.f, "%.1f pixels" );
  auto &pc_detection_scale = m_reg->ctx().get<Cmp::Persistent::PCDetectionScale>();
  ImGui::SliderFloat( "PC Detection Bounding Box Scale Factor", &pc_detection_scale(), 1.f, 20.f, "%.1f pixels" );

  auto &npc_lerp_speed = m_reg->ctx().get<Cmp::Persistent::NpcLerpSpeed>();
  ImGui::SliderFloat( "NPC Speed", &npc_lerp_speed(), 0.1f, 3.f, "%.1f" );

  auto &npc_damage_cooldown = m_reg->ctx().get<Cmp::Persistent::NpcDamageDelay>();
  ImGui::SliderFloat( "NPC Damage Cooldown", &npc_damage_cooldown(), 0.1f, 2.f, "%.1f seconds" );

  auto &music_volume = m_reg->ctx().get<Cmp::Persistent::MusicVolume>();
  ImGui::SliderFloat( "Music Volume", &music_volume(), 0.f, 100.f, "%.1f" );

  ImGui::End();
  ImGui::SFML::Render( getWindow() );
}

void RenderMenuSystem::render_paused()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "Paused", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press P to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_defeat_screen()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "You died!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

void RenderMenuSystem::render_victory_screen()
{
  // main render begin
  getWindow().clear();
  {
    sf::Text title_text( m_font, "You won!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    getWindow().draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    getWindow().draw( start_text );
  }

  getWindow().display();
  // main render end
}

} // namespace ProceduralMaze::Sys