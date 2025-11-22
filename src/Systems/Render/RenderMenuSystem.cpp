#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
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
#include <Components/Persistent/ShrineCostPerSprite.hpp>
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys
{

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
  try
  {
    // Player Settings

    ImGui::SeparatorText( "Player Settings" );

    auto &bomb_inventory = get_persistent_component<Cmp::Persistent::BombInventory>();
    ImGui::SliderScalar( "Bomb Inventory", ImGuiDataType_U8, bomb_inventory.get_value_ptr(), bomb_inventory.get_min_value_ptr(),
                         bomb_inventory.get_max_value_ptr(), "%d" );

    auto &blast_radius = get_persistent_component<Cmp::Persistent::BlastRadius>();
    ImGui::SliderScalar( "Blast Radius", ImGuiDataType_U8, blast_radius.get_value_ptr(), blast_radius.get_min_value_ptr(),
                         blast_radius.get_max_value_ptr(), "%d" );

    auto &player_anim_framerate = get_persistent_component<Cmp::Persistent::PlayerAnimFramerate>();
    ImGui::SliderScalar( "Player Animation Framerate", ImGuiDataType_Float, player_anim_framerate.get_value_ptr(),
                         player_anim_framerate.get_min_value_ptr(), player_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &player_lerp_speed = get_persistent_component<Cmp::Persistent::PlayerLerpSpeed>();
    ImGui::SliderScalar( "Player Lerp Speed", ImGuiDataType_Float, player_lerp_speed.get_value_ptr(),
                         player_lerp_speed.get_min_value_ptr(), player_lerp_speed.get_max_value_ptr(), "%.1f" );

    auto &player_diagonal_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>();
    ImGui::SliderScalar(
        "Player Diagonal Lerp Speed Modifier", ImGuiDataType_Float, player_diagonal_lerp_speed_modifier.get_value_ptr(),
        player_diagonal_lerp_speed_modifier.get_min_value_ptr(), player_diagonal_lerp_speed_modifier.get_max_value_ptr(), "%.2f" );

    auto &player_shortcut_lerp_speed_modifier = get_persistent_component<Cmp::Persistent::PlayerShortcutLerpSpeedModifier>();
    ImGui::SliderScalar(
        "Player Shortcut Lerp Speed Modifier", ImGuiDataType_Float, player_shortcut_lerp_speed_modifier.get_value_ptr(),
        player_shortcut_lerp_speed_modifier.get_min_value_ptr(), player_shortcut_lerp_speed_modifier.get_max_value_ptr(), "%.2f" );

    auto &digging_cooldown = get_persistent_component<Cmp::Persistent::DiggingCooldownThreshold>();
    ImGui::SliderScalar( "Digging Cooldown", ImGuiDataType_Float, digging_cooldown.get_value_ptr(),
                         digging_cooldown.get_min_value_ptr(), digging_cooldown.get_max_value_ptr(), "%.2f seconds" );

    auto &digging_damage_per_hit = get_persistent_component<Cmp::Persistent::DiggingDamagePerHit>();
    ImGui::SliderScalar( "Digging Damage Per Hit", ImGuiDataType_Float, digging_damage_per_hit.get_value_ptr(),
                         digging_damage_per_hit.get_min_value_ptr(), digging_damage_per_hit.get_max_value_ptr(), "%.2f damage" );

    auto &weapon_degrade_per_hit = get_persistent_component<Cmp::Persistent::WeaponDegradePerHit>();
    ImGui::SliderScalar( "Weapon Degrade Per Hit", ImGuiDataType_Float, weapon_degrade_per_hit.get_value_ptr(),
                         weapon_degrade_per_hit.get_min_value_ptr(), weapon_degrade_per_hit.get_max_value_ptr(), "%.2f level" );

    auto &pc_detection_scale = get_persistent_component<Cmp::Persistent::PlayerDetectionScale>();
    ImGui::SliderScalar( "PC Detection Bounding Box Scale Factor", ImGuiDataType_Float, pc_detection_scale.get_value_ptr(),
                         pc_detection_scale.get_min_value_ptr(), pc_detection_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &pc_damage_cooldown = get_persistent_component<Cmp::Persistent::PcDamageDelay>();
    ImGui::SliderScalar( "PC Damage Cooldown", ImGuiDataType_Float, pc_damage_cooldown.get_value_ptr(),
                         pc_damage_cooldown.get_min_value_ptr(), pc_damage_cooldown.get_max_value_ptr(), "%.1f seconds" );

    // Bomb Settings
    ImGui::SeparatorText( "Bomb Settings" );

    auto &bomb_damage = get_persistent_component<Cmp::Persistent::BombDamage>();
    ImGui::SliderScalar( "Bomb Damage", ImGuiDataType_U8, bomb_damage.get_value_ptr(), bomb_damage.get_min_value_ptr(),
                         bomb_damage.get_max_value_ptr(), "%d" );

    auto &fuse_delay = get_persistent_component<Cmp::Persistent::FuseDelay>();
    ImGui::SliderScalar( "Fuse Delay", ImGuiDataType_Float, fuse_delay.get_value_ptr(), fuse_delay.get_min_value_ptr(),
                         fuse_delay.get_max_value_ptr(), "%.1f seconds" );

    auto &armed_on_delay = get_persistent_component<Cmp::Persistent::ArmedOnDelay>();
    ImGui::SliderScalar( "Armed Detonation Delay Increment", ImGuiDataType_Float, armed_on_delay.get_value_ptr(),
                         armed_on_delay.get_min_value_ptr(), armed_on_delay.get_max_value_ptr(), "%.3f" );

    auto &armed_off_delay = get_persistent_component<Cmp::Persistent::ArmedOffDelay>();
    ImGui::SliderScalar( "Armed Off Delay", ImGuiDataType_Float, armed_off_delay.get_value_ptr(),
                         armed_off_delay.get_min_value_ptr(), armed_off_delay.get_max_value_ptr(), "%.3f" );

    // Hazard Settings
    ImGui::SeparatorText( "Hazard Settings" );
    auto &corruption_damage = get_persistent_component<Cmp::Persistent::CorruptionDamage>();
    ImGui::SliderScalar( "Corruption Damage", ImGuiDataType_U8, corruption_damage.get_value_ptr(),
                         corruption_damage.get_min_value_ptr(), corruption_damage.get_max_value_ptr(), "%d" );

    auto &wormhole_anim_framerate = get_persistent_component<Cmp::Persistent::WormholeAnimFramerate>();
    ImGui::SliderScalar( "Wormhole Animation Framerate", ImGuiDataType_Float, wormhole_anim_framerate.get_value_ptr(),
                         wormhole_anim_framerate.get_min_value_ptr(), wormhole_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &wormhole_seed = get_persistent_component<Cmp::Persistent::WormholeSeed>();
    ImGui::InputScalar( "Wormhole Seed. Zero is ignored", ImGuiDataType_U64, wormhole_seed.get_value_ptr(), nullptr, nullptr,
                        "%llu", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    auto &corruption_seed = get_persistent_component<Cmp::Persistent::CorruptionSeed>();
    ImGui::InputScalar( "Corruption Seed. Zero is ignored", ImGuiDataType_U64, corruption_seed.get_value_ptr(), nullptr, nullptr,
                        "%llu", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    auto &sinkhole_seed = get_persistent_component<Cmp::Persistent::SinkholeSeed>();
    ImGui::InputScalar( "Sinkhole Seed. Zero is ignored", ImGuiDataType_U64, sinkhole_seed.get_value_ptr(), nullptr, nullptr,
                        "%llu", ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    // Loot Settings
    ImGui::SeparatorText( "Loot Settings" );
    auto &health_bonus = get_persistent_component<Cmp::Persistent::HealthBonus>();
    ImGui::SliderScalar( "Health Bonus", ImGuiDataType_U8, health_bonus.get_value_ptr(), health_bonus.get_min_value_ptr(),
                         health_bonus.get_max_value_ptr(), "%d" );

    auto &bomb_bonus = get_persistent_component<Cmp::Persistent::BombBonus>();
    ImGui::SliderScalar( "Bomb Bonus", ImGuiDataType_U8, bomb_bonus.get_value_ptr(), bomb_bonus.get_min_value_ptr(),
                         bomb_bonus.get_max_value_ptr(), "%d" );

    // NPC Settings
    ImGui::SeparatorText( "NPC Settings" );

    auto &npc_anim_framerate = get_persistent_component<Cmp::Persistent::NpcSkeleAnimFramerate>();
    ImGui::SliderScalar( "NPC Animation Skeleton Framerate", ImGuiDataType_Float, npc_anim_framerate.get_value_ptr(),
                         npc_anim_framerate.get_min_value_ptr(), npc_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_ghost_anim_framerate = get_persistent_component<Cmp::Persistent::NpcGhostAnimFramerate>();
    ImGui::SliderScalar( "NPC Animation Ghost Framerate", ImGuiDataType_Float, npc_ghost_anim_framerate.get_value_ptr(),
                         npc_ghost_anim_framerate.get_min_value_ptr(), npc_ghost_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_damage = get_persistent_component<Cmp::Persistent::NpcDamage>();
    ImGui::SliderScalar( "NPC Damage", ImGuiDataType_U8, npc_damage.get_value_ptr(), npc_damage.get_min_value_ptr(),
                         npc_damage.get_max_value_ptr(), "%d" );

    auto &npc_push_back = get_persistent_component<Cmp::Persistent::NpcPushBack>();
    if ( ImGui::SliderScalar( "NPC Push Back Distance", ImGuiDataType_Float, npc_push_back.get_value_ptr(),
                              npc_push_back.get_min_value_ptr(), npc_push_back.get_max_value_ptr(), "%.1f pixels" ) )
    {
      float step = kGridSquareSizePixelsF.x;
      npc_push_back.get_value() = roundf( npc_push_back.get_value() / step ) * step;
    }

    auto &npc_death_anim_framerate = get_persistent_component<Cmp::Persistent::NpcDeathAnimFramerate>();
    ImGui::SliderScalar( "NPC Death Animation Framerate", ImGuiDataType_Float, npc_death_anim_framerate.get_value_ptr(),
                         npc_death_anim_framerate.get_min_value_ptr(), npc_death_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_activate_scale = get_persistent_component<Cmp::Persistent::NpcActivateScale>();
    ImGui::SliderScalar( "NPC Activation Bounding Box Scale Factor", ImGuiDataType_Float, npc_activate_scale.get_value_ptr(),
                         npc_activate_scale.get_min_value_ptr(), npc_activate_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &npc_scan_scale = get_persistent_component<Cmp::Persistent::NpcScanScale>();
    ImGui::SliderScalar( "NPC Scan Bounding Box Scale Factor", ImGuiDataType_Float, npc_scan_scale.get_value_ptr(),
                         npc_scan_scale.get_min_value_ptr(), npc_scan_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &npc_lerp_speed = get_persistent_component<Cmp::Persistent::NpcLerpSpeed>();
    ImGui::SliderScalar( "NPC Speed", ImGuiDataType_Float, npc_lerp_speed.get_value_ptr(), npc_lerp_speed.get_min_value_ptr(),
                         npc_lerp_speed.get_max_value_ptr(), "%.1f" );

    // Audio
    ImGui::SeparatorText( "Audio" );

    auto &music_volume = get_persistent_component<Cmp::Persistent::MusicVolume>();
    ImGui::SliderScalar( "Music Volume", ImGuiDataType_Float, music_volume.get_value_ptr(), music_volume.get_min_value_ptr(),
                         music_volume.get_max_value_ptr(), "%.1f" );

    auto &effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
    ImGui::SliderScalar( "Effects Volume", ImGuiDataType_Float, effects_volume.get_value_ptr(), effects_volume.get_min_value_ptr(),
                         effects_volume.get_max_value_ptr(), "%.1f" );

    // Procedural Generation
    ImGui::SeparatorText( "Procedural Generation" );

    auto &max_shrines = get_persistent_component<Cmp::Persistent::MaxShrines>();
    ImGui::SliderScalar( "Max Shrines", ImGuiDataType_U8, max_shrines.get_value_ptr(), max_shrines.get_min_value_ptr(),
                         max_shrines.get_max_value_ptr(), "%d Max Shrines" );

    auto &grave_num_multiplier = get_persistent_component<Cmp::Persistent::GraveNumMultiplier>();
    ImGui::SliderScalar( "Grave Number Multiplier", ImGuiDataType_U8, grave_num_multiplier.get_value_ptr(),
                         grave_num_multiplier.get_min_value_ptr(), grave_num_multiplier.get_max_value_ptr(), "%d" );

    auto &shrine_cost = get_persistent_component<Cmp::Persistent::ShrineCostPerSprite>();
    ImGui::SliderScalar( "Shrine Cost", ImGuiDataType_U8, shrine_cost.get_value_ptr(), shrine_cost.get_min_value_ptr(),
                         shrine_cost.get_max_value_ptr(), "%d Shrine Cost per sprite" );
  }
  catch ( const std::exception &e )
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

  sf::Text title_text( m_font, "Paused", 96 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press P to continue", 48 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
  m_window.draw( start_text );

  auto &music_volume = get_persistent_component<Cmp::Persistent::MusicVolume>();
  ImGui::SliderFloat( "Music Volume", &music_volume.get_value(), 0.f, 100.f, "%.1f" );

  auto &effects_volume = get_persistent_component<Cmp::Persistent::EffectsVolume>();
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