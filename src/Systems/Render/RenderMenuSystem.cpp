#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <SFML/System/Angle.hpp>

#include <imgui.h>

#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxShrines.hpp>
#include <Components/Persistent/MusicVolume.hpp>
#include <Components/Persistent/NpcActivateScale.hpp>
#include <Components/Persistent/NpcAnimFramerate.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcDeathAnimFramerate.hpp>
#include <Components/Persistent/NpcGhostAnimFramerate.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/NpcScanScale.hpp>
#include <Components/Persistent/NpcSkeleAnimFramerate.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/PlayerAnimFramerate.hpp>
#include <Components/Persistent/PlayerDiagonalLerpSpeedModifier.hpp>
#include <Components/Persistent/PlayerLerpSpeed.hpp>
#include <Components/Persistent/PlayerShortcutLerpSpeedModifier.hpp>
#include <Components/Persistent/ShrineCost.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WaterBonus.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys {

void RenderMenuSystem::init_title()
{
  m_title_screen_shader = std::make_unique<Sprites::TitleScreenShader>( "res/shaders/TitleScreen.frag",
                                                                        Sys::BaseSystem::kDisplaySize );
  m_title_screen_shader->setup();
}

void RenderMenuSystem::render_title()
{
  // main render begin
  m_window.clear();
  {
    // shaders
    m_title_screen_shader->set_position( { 0, 0 } );
    const auto mouse_pos = sf::Vector2f( sf::Mouse::getPosition( m_window ) )
                               .componentWiseDiv( sf::Vector2f( m_window.getSize() ) );
    m_title_screen_shader->update( mouse_pos );
    m_window.draw( *m_title_screen_shader );

    // text
    sf::Color txt_color{ 64, 96, 184 };
    sf::Color txt_outline_color{ 48, 48, 48 };

    render_text( "Resurrectionist", 128, { kDisplaySize.x * 0.25f, 200.f }, Alignment::CENTER, 2.f, txt_outline_color );

    render_text( "Press <Enter> key to start", 48, { kDisplaySize.x * 0.25f, 400.f }, Alignment::CENTER, 3.f, txt_color );

    render_text( "Press <Q> key to quit", 48, { kDisplaySize.x * 0.25f, 500.f }, Alignment::CENTER, 3.f, txt_color );

    render_text( "Press <S> key for settings", 48, { kDisplaySize.x * 0.25f, 600.f }, Alignment::CENTER, 3.f, txt_color );
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_settings( sf::Time globalDeltaTime )
{
  m_window.clear();

  sf::Text title_text( m_font, "Settings", 64 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { 100.f, 50.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press <Esc> key to go back", 24 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { 400.f, 95.f } );
  m_window.draw( start_text );

  // ImGUI should be rendered before window.display() or SFML wipes the display buffer prematurely
  render_settings_widgets( globalDeltaTime );

  m_window.display();
}

void RenderMenuSystem::render_settings_widgets( sf::Time globalDeltaTime )
{
  // need to make sure we call Update() and Render() every frame
  ImGui::SFML::Update( m_window, globalDeltaTime );
  ImGui::Begin( "Settings", nullptr, kImGuiWindowOptions );

  // Player Settings
  ImGui::SeparatorText( "Player Settings" );
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

  auto &player_diagonal_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Diagonal Lerp Speed Modifier", &player_diagonal_lerp_speed_modifier.get_value(), 0.001f, 1.f,
                      "%.2f" );

  auto &player_shortcut_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
  ImGui::SliderFloat( "Player Shortcut Lerp Speed Modifier", &player_shortcut_lerp_speed_modifier.get_value(), 0.001f, 1.f,
                      "%.2f" );

  auto &digging_cooldown = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>();
  ImGui::SliderFloat( "Digging Cooldown", &digging_cooldown.get_value(), 0.05f, 1.f, "%.2f seconds" );

  auto &digging_damage_per_hit = get_persistent_component<Cmp::Persistent::DiggingDamagePerHit>();
  ImGui::SliderFloat( "Digging Damage Per Hit", &digging_damage_per_hit.get_value(), 0.01f, 1.0f, "%.2f damage" );

  auto &weapon_degrade_per_hit = get_persistent_component<Cmp::Persistent::WeaponDegradePerHit>();
  ImGui::SliderFloat( "Weapon Degrade Per Hit", &weapon_degrade_per_hit.get_value(), 0.01f, 5.0f, "%.2f level" );

  auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();
  ImGui::SliderFloat( "PC Detection Bounding Box Scale Factor", &pc_detection_scale.get_value(), 1.f, 20.f, "%.1f pixels" );
  auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
  ImGui::SliderFloat( "PC Damage Cooldown", &pc_damage_cooldown.get_value(), 0.1f, 2.f, "%.1f seconds" );

  // Bomb Settings
  ImGui::SeparatorText( "Bomb Settings" );

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

  // Hazard Settings
  ImGui::SeparatorText( "Hazard Settings" );
  auto &wormhole_anim_framerate = get_persistent_component<Cmp::Persistent::WormholeAnimFramerate>();
  ImGui::SliderFloat( "Wormhole Animation Framerate", &wormhole_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &wormhole_seed = get_persistent_component<Cmp::Persistent::WormholeSeed>();
  ImGui::InputScalar( "Wormhole Seed. Zero is ignored", ImGuiDataType_U64, &wormhole_seed.get_value() );
  auto &corruption_seed = get_persistent_component<Cmp::Persistent::CorruptionSeed>();
  ImGui::InputScalar( "Corruption Seed. Zero is ignored", ImGuiDataType_U64, &corruption_seed.get_value() );
  auto &sinkhole_seed = get_persistent_component<Cmp::Persistent::SinkholeSeed>();
  ImGui::InputScalar( "Sinkhole Seed. Zero is ignored", ImGuiDataType_U64, &sinkhole_seed.get_value() );

  // Loot Settings
  ImGui::SeparatorText( "Loot Settings" );
  auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
  ImGui::SliderInt( "Health Bonus", &health_bonus.get_value(), 1, 50 );
  auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
  ImGui::SliderInt( "Bomb Bonus", &bomb_bonus.get_value(), 1, 20 );
  auto &water_bonus = get_persistent_component<Cmp::Persistent::WaterBonus>();
  ImGui::SliderFloat( "Water Bonus", &water_bonus.get_value(), 10.f, 500.f, "%.1f units" );

  // NPC Settings
  ImGui::SeparatorText( "NPC Settings" );

  auto &npc_anim_framerate = get_persistent_component<Cmp::Persistent::NpcSkeleAnimFramerate>();
  ImGui::SliderFloat( "NPC Animation Skeleton Framerate", &npc_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &npc_ghost_anim_framerate = get_persistent_component<Cmp::Persistent::NpcGhostAnimFramerate>();
  ImGui::SliderFloat( "NPC Animation Ghost Framerate", &npc_ghost_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &npc_damage = get_persistent_component<Cmp::Persistent::NpcDamage>();
  ImGui::SliderInt( "NPC Damage", &npc_damage.get_value(), 1, 50 );

  auto &npc_push_back = get_persistent_component<Cmp::Persistent::NpcPushBack>();
  if ( ImGui::InputFloat( "NPC Push Back Distance", &npc_push_back.get_value(), 0.1f, 0.1f, "%.2f pixels" ) )
  {
    npc_push_back.get_value() = std::clamp( npc_push_back.get_value(), 1.0f, 50.0f );
  }

  auto &npc_death_anim_framerate = get_persistent_component<Cmp::Persistent::NpcDeathAnimFramerate>();
  ImGui::SliderFloat( "NPC Death Animation Framerate", &npc_death_anim_framerate.get_value(), 0.01f, 0.5f, "%.2f" );

  auto &npc_activate_scale = get_persistent_component<Cmp::Persistent::NpcActivateScale>();
  ImGui::SliderFloat( "NPC Activation Bounding Box Scale Factor", &npc_activate_scale.get_value(), 1.f, 20.f, "%.1f pixels" );
  auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
  ImGui::SliderFloat( "NPC Scan Bounding Box Scale Factor", &npc_scan_scale.get_value(), 1.f, 3.f, "%.1f pixels" );

  auto &npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
  ImGui::SliderFloat( "NPC Speed", &npc_lerp_speed.get_value(), 0.1f, 3.f, "%.1f" );

  // Audio
  ImGui::SeparatorText( "Audio" );

  auto &music_volume = get_persistent_component<Cmp::Persistent::MusicVolume>();
  ImGui::SliderFloat( "Music Volume", &music_volume.get_value(), 0.f, 100.f, "%.1f" );

  auto &effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
  ImGui::SliderFloat( "Effects Volume", &effects_volume.get_value(), 0.f, 100.f, "%.1f" );

  // Procedural Generation
  ImGui::SeparatorText( "Procedural Generation" );

  auto &max_shrines = get_persistent_component<Cmp::Persistent::MaxShrines>();
  ImGui::SliderInt( "Max Shrines", reinterpret_cast<int *>( &max_shrines.get_value() ), 1, 10 );
  auto &shrine_cost = get_persistent_component<Cmp::Persistent::ShrineCost>();
  ImGui::SliderInt( "Shrine Cost", reinterpret_cast<int *>( &shrine_cost.get_value() ), 1, 10 );

  ImGui::End();
  ImGui::SFML::Render( m_window );
}

void RenderMenuSystem::render_paused()
{
  // main render begin
  m_window.clear();
  {
    sf::Text title_text( m_font, "Paused", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    m_window.draw( title_text );

    sf::Text start_text( m_font, "Press P to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    m_window.draw( start_text );
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_defeat_screen()
{
  // main render begin
  m_window.clear();
  {
    sf::Text title_text( m_font, "You died!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    m_window.draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    m_window.draw( start_text );
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_victory_screen()
{
  // main render begin
  m_window.clear();
  {
    sf::Text title_text( m_font, "You won!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    m_window.draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    m_window.draw( start_text );
  }

  m_window.display();
  // main render end
}

} // namespace ProceduralMaze::Sys