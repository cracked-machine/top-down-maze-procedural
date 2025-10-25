#include <BombSystem.hpp>
#include <Persistent/ArmedOnDelay.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/BombDamage.hpp>
#include <Persistent/CorruptionSeed.hpp>
#include <Persistent/DiggingCooldownThreshold.hpp>
#include <Persistent/DiggingDamagePerHit .hpp>
#include <Persistent/FuseDelay.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/MaxShrines.hpp>
#include <Persistent/MusicVolume.hpp>
#include <Persistent/NpcActivateScale.hpp>
#include <Persistent/NpcAnimFramerate.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcDeathAnimFramerate.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/NpcScanScale.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/PlayerAnimFramerate.hpp>
#include <Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Persistent/PlayerLerpSpeed.hpp>
#include <Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Persistent/PlayerSubmergedlLerpSpeedModifier.hpp>
#include <Persistent/ShrineCost.hpp>
#include <Persistent/SinkholeSeed.hpp>
#include <Persistent/WaterBonus.hpp>
#include <Persistent/WormholeAnimFramerate.hpp>
#include <Persistent/WormholeSeed.hpp>
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

  auto &bomb_inventory = get_persistent_component<Cmp::Persistent::BombInventory>();
  if ( ImGui::InputInt( "Bomb Inventory", &bomb_inventory.get_value() ) )
  {
    bomb_inventory.get_value() = std::clamp( bomb_inventory.get_value(), -1, 100 );
  }

  auto &blast_radius = get_persistent_component<Cmp::Persistent::BlastRadius>();
  ImGui::SliderInt( "Blast Radius", &blast_radius.get_value(), 1, 3 );

  auto &player_anim_framerate = get_persistent_component<Cmp::Persistent::PlayerAnimFramerate>();
  ImGui::SliderFloat( "Player Animation Framerate", &player_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &player_lerp_speed = get_persistent_component<Cmp::Persistent::PlayerLerpSpeed>();
  ImGui::SliderFloat( "Player Lerp Speed", &player_lerp_speed.get_value(), 3.f, 10.f, "%.1f" );

  auto &player_diagonal_lerp_speed_modifier = get_persistent_component<
      Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Diagonal Lerp Speed Modifier", &player_diagonal_lerp_speed_modifier.get_value(), 0.001f,
                      1.f, "%.2f" );

  auto &player_shortcut_lerp_speed_modifier = get_persistent_component<
      Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Shortcut Lerp Speed Modifier", &player_shortcut_lerp_speed_modifier.get_value(), 0.001f,
                      1.f, "%.2f" );

  auto &player_submerged_lerp_speed_modifier = get_persistent_component<
      Cmp::Persistent::PlayerSubmergedLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Submerged Lerp Speed Modifier", &player_submerged_lerp_speed_modifier.get_value(), 0.001f,
                      1.f, "%.2f" );

  auto &digging_cooldown = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>();
  ImGui::SliderFloat( "Digging Cooldown", &digging_cooldown.get_value(), 0.05f, 1.f, "%.2f seconds" );

  auto &digging_damage_per_hit = get_persistent_component<Cmp::Persistent::DiggingDamagePerHit>();
  ImGui::SliderFloat( "Digging Damage Per Hit", &digging_damage_per_hit.get_value(), 0.01f, 1.0f, "%.2f damage" );

  ImGui::Separator();
  auto &flood_speed = get_persistent_component<Cmp::Persistent::FloodSpeed>();
  ImGui::SliderFloat( "Flood Velocity", &flood_speed.get_value(), 1.f, 10.f, "%.1f pixels/second" );
  ImGui::Separator();

  auto &npc_anim_framerate = get_persistent_component<Cmp::Persistent::NpcAnimFramerate>();
  ImGui::SliderFloat( "NPC Animation Framerate", &npc_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &npc_damage = get_persistent_component<Cmp::Persistent::NpcDamage>();
  ImGui::SliderInt( "NPC Damage", &npc_damage.get_value(), 1, 50 );

  auto &bomb_damage = get_persistent_component<Cmp::Persistent::BombDamage>();
  ImGui::SliderInt( "Bomb Damage", &bomb_damage.get_value(), 1, 50 );

  auto &fuse_delay = get_persistent_component<Cmp::Persistent::FuseDelay>();
  ImGui::SliderFloat( "Fuse Delay", &fuse_delay.get_value(), 1.f, 10.f, "%.1f seconds" );

  auto &armed_on_delay = get_persistent_component<Cmp::Persistent::ArmedOnDelay>();
  if ( ImGui::InputFloat( "Armed Detonation Delay Increment", &armed_on_delay.get_value(), 0.001f, 0.001f ) )
  {
    armed_on_delay.get_value() = std::clamp( armed_on_delay.get_value(), 0.001f, 0.5f );
  }
  auto &armed_off_delay = get_persistent_component<Cmp::Persistent::ArmedOffDelay>();
  if ( ImGui::InputFloat( "Armed Off Delay", &armed_off_delay.get_value(), 0.001f, 0.001f ) )
  {
    armed_off_delay.get_value() = std::clamp( armed_off_delay.get_value(), 0.001f, 0.5f );
  }

  ImGui::Separator();
  auto &wormhole_anim_framerate = get_persistent_component<Cmp::Persistent::WormholeAnimFramerate>();
  ImGui::SliderFloat( "Wormhole Animation Framerate", &wormhole_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &wormhole_seed = get_persistent_component<Cmp::Persistent::WormholeSeed>();
  ImGui::InputScalar( "Wormhole Seed. Zero is ignored", ImGuiDataType_U64, &wormhole_seed.get_value() );
  auto &corruption_seed = get_persistent_component<Cmp::Persistent::CorruptionSeed>();
  ImGui::InputScalar( "Corruption Seed. Zero is ignored", ImGuiDataType_U64, &corruption_seed.get_value() );
  auto &sinkhole_seed = get_persistent_component<Cmp::Persistent::SinkholeSeed>();
  ImGui::InputScalar( "Sinkhole Seed. Zero is ignored", ImGuiDataType_U64, &sinkhole_seed.get_value() );

  auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
  ImGui::SliderInt( "Health Bonus", &health_bonus.get_value(), 1, 50 );
  auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
  ImGui::SliderInt( "Bomb Bonus", &bomb_bonus.get_value(), 1, 20 );
  auto &water_bonus = get_persistent_component<Cmp::Persistent::WaterBonus>();
  ImGui::SliderFloat( "Water Bonus", &water_bonus.get_value(), 10.f, 500.f, "%.1f units" );

  auto &obstacle_push_back = get_persistent_component<Cmp::Persistent::ObstaclePushBack>();
  if ( ImGui::InputFloat( "Obstacle Push Back Factor", &obstacle_push_back.get_value(), 0.1f, 0.1f, "%.2f" ) )
  {
    obstacle_push_back.get_value() = std::clamp( obstacle_push_back.get_value(), 1.0f, 5.0f );
  }
  auto &npc_push_back = get_persistent_component<Cmp::Persistent::NpcPushBack>();
  if ( ImGui::InputFloat( "NPC Push Back Distance", &npc_push_back.get_value(), 0.1f, 0.1f, "%.2f pixels" ) )
  {
    npc_push_back.get_value() = std::clamp( npc_push_back.get_value(), 1.0f, 50.0f );
  }

  auto &npc_death_anim_framerate = get_persistent_component<Cmp::Persistent::NpcDeathAnimFramerate>();
  ImGui::SliderFloat( "NPC Death Animation Framerate", &npc_death_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &npc_activate_scale = get_persistent_component<Cmp::Persistent::NpcActivateScale>();
  ImGui::SliderFloat( "NPC Activation Bounding Box Scale Factor", &npc_activate_scale.get_value(), 1.f, 20.f,
                      "%.1f pixels" );
  auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
  ImGui::SliderFloat( "NPC Scan Bounding Box Scale Factor", &npc_scan_scale.get_value(), 1.f, 3.f, "%.1f pixels" );
  auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();
  ImGui::SliderFloat( "PC Detection Bounding Box Scale Factor", &pc_detection_scale.get_value(), 1.f, 20.f,
                      "%.1f pixels" );

  auto &npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
  ImGui::SliderFloat( "NPC Speed", &npc_lerp_speed.get_value(), 0.1f, 3.f, "%.1f" );

  auto &npc_damage_cooldown = get_persistent_component<Cmp::Persistent::NpcDamageDelay>();
  ImGui::SliderFloat( "NPC Damage Cooldown", &npc_damage_cooldown.get_value(), 0.1f, 2.f, "%.1f seconds" );

  auto &music_volume = get_persistent_component<Cmp::Persistent::MusicVolume>();
  ImGui::SliderFloat( "Music Volume", &music_volume.get_value(), 0.f, 100.f, "%.1f" );

  auto &max_shrines = get_persistent_component<Cmp::Persistent::MaxShrines>();
  ImGui::SliderInt( "Max Shrines", reinterpret_cast<int *>( &max_shrines.get_value() ), 1, 10 );
  auto &shrine_cost = get_persistent_component<Cmp::Persistent::ShrineCost>();
  ImGui::SliderInt( "Shrine Cost", reinterpret_cast<int *>( &shrine_cost.get_value() ), 1, 10 );

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