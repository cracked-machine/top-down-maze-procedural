#include <Components/Persistent/BlastRadius.hpp>
#include <Components/Persistent/BombInventory.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/EffectsVolume.hpp>
#include <Components/Persistent/ExitKeyRequirement.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/Persistent/PlayerDetectionScale.hpp>
#include <Components/Persistent/PlayerFootstepAddDelay.hpp>
#include <Components/Persistent/PlayerFootstepFadeDelay.hpp>
#include <Components/Persistent/WeaponDegradePerHit.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SFML/System/Angle.hpp>

#include <Systems/PersistentSystem.hpp>
#include <imgui.h>

#include <Components/Persistent/ArmedOnDelay.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/BombDamage.hpp>
#include <Components/Persistent/CorruptionSeed.hpp>
#include <Components/Persistent/DiggingCooldownThreshold.hpp>
#include <Components/Persistent/DiggingDamagePerHit.hpp>
#include <Components/Persistent/FuseDelay.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
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
#include <Components/Persistent/SinkholeSeed.hpp>
#include <Components/Persistent/WormholeAnimFramerate.hpp>
#include <Components/Persistent/WormholeSeed.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Sys
{

void RenderMenuSystem::init_title()
{
  m_title_screen_shader = std::make_unique<Sprites::TitleScreenShader>(
      "res/shaders/TitleScreen.frag", Sys::BaseSystem::kDisplaySize );
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

    render_text( "Resurrectionist", 128, { kDisplaySize.x * 0.25f, 200.f }, Alignment::CENTER, 2.f,
                 txt_outline_color );

    render_text( "Press <Enter> key to start", 48, { kDisplaySize.x * 0.25f, 400.f },
                 Alignment::CENTER, 3.f, txt_color );

    render_text( "Press <Q> key to quit", 48, { kDisplaySize.x * 0.25f, 500.f }, Alignment::CENTER,
                 3.f, txt_color );

    render_text( "Press <S> key for settings", 48, { kDisplaySize.x * 0.25f, 600.f },
                 Alignment::CENTER, 3.f, txt_color );
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

    auto &bomb_inventory = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::BombInventory>(
        getReg() );
    ImGui::SliderScalar( "Bomb Inventory", ImGuiDataType_U8, bomb_inventory.get_value_ptr(),
                         bomb_inventory.get_min_value_ptr(), bomb_inventory.get_max_value_ptr(),
                         "%d" );

    auto &blast_radius = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::BlastRadius>(
        getReg() );
    ImGui::SliderScalar( "Blast Radius", ImGuiDataType_U8, blast_radius.get_value_ptr(),
                         blast_radius.get_min_value_ptr(), blast_radius.get_max_value_ptr(), "%d" );

    auto &player_anim_framerate = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerAnimFramerate>( getReg() );
    ImGui::SliderScalar( "Player Animation Framerate", ImGuiDataType_Float,
                         player_anim_framerate.get_value_ptr(),
                         player_anim_framerate.get_min_value_ptr(),
                         player_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &player_lerp_speed = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerLerpSpeed>( getReg() );
    ImGui::SliderScalar( "Player Lerp Speed", ImGuiDataType_Float,
                         player_lerp_speed.get_value_ptr(), player_lerp_speed.get_min_value_ptr(),
                         player_lerp_speed.get_max_value_ptr(), "%.1f" );

    auto &player_diagonal_lerp_speed_modifier = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerDiagonalLerpSpeedModifier>( getReg() );
    ImGui::SliderScalar( "Player Diagonal Lerp Speed Modifier", ImGuiDataType_Float,
                         player_diagonal_lerp_speed_modifier.get_value_ptr(),
                         player_diagonal_lerp_speed_modifier.get_min_value_ptr(),
                         player_diagonal_lerp_speed_modifier.get_max_value_ptr(), "%.2f" );

    auto &player_footstep_add_delay = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerFootstepAddDelay>( getReg() );
    ImGui::SliderScalar( "Player Footstep Add Delay", ImGuiDataType_Float,
                         player_footstep_add_delay.get_value_ptr(),
                         player_footstep_add_delay.get_min_value_ptr(),
                         player_footstep_add_delay.get_max_value_ptr(), "%.2f seconds" );

    auto &player_footstep_fade_delay = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerFootstepFadeDelay>( getReg() );
    ImGui::SliderScalar( "Player Footstep Fade Delay", ImGuiDataType_Float,
                         player_footstep_fade_delay.get_value_ptr(),
                         player_footstep_fade_delay.get_min_value_ptr(),
                         player_footstep_fade_delay.get_max_value_ptr(), "%.2f seconds" );

    auto &player_shortcut_lerp_speed_modifier = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerShortcutLerpSpeedModifier>( getReg() );
    ImGui::SliderScalar( "Player Shortcut Lerp Speed Modifier", ImGuiDataType_Float,
                         player_shortcut_lerp_speed_modifier.get_value_ptr(),
                         player_shortcut_lerp_speed_modifier.get_min_value_ptr(),
                         player_shortcut_lerp_speed_modifier.get_max_value_ptr(), "%.2f" );

    auto &digging_cooldown = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::DiggingCooldownThreshold>( getReg() );
    ImGui::SliderScalar( "Digging Cooldown", ImGuiDataType_Float, digging_cooldown.get_value_ptr(),
                         digging_cooldown.get_min_value_ptr(), digging_cooldown.get_max_value_ptr(),
                         "%.2f seconds" );

    auto &digging_damage_per_hit = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::DiggingDamagePerHit>( getReg() );
    ImGui::SliderScalar( "Digging Damage Per Hit", ImGuiDataType_U8,
                         digging_damage_per_hit.get_value_ptr(),
                         digging_damage_per_hit.get_min_value_ptr(),
                         digging_damage_per_hit.get_max_value_ptr(), "%d damage" );

    auto &weapon_degrade_per_hit = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::WeaponDegradePerHit>( getReg() );
    ImGui::SliderScalar( "Weapon Degrade Per Hit", ImGuiDataType_Float,
                         weapon_degrade_per_hit.get_value_ptr(),
                         weapon_degrade_per_hit.get_min_value_ptr(),
                         weapon_degrade_per_hit.get_max_value_ptr(), "%.2f level" );

    auto &pc_detection_scale = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::PlayerDetectionScale>( getReg() );
    ImGui::SliderScalar( "PC Detection Bounding Box Scale Factor", ImGuiDataType_Float,
                         pc_detection_scale.get_value_ptr(), pc_detection_scale.get_min_value_ptr(),
                         pc_detection_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &
        pc_damage_cooldown = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::PcDamageDelay>(
            getReg() );
    ImGui::SliderScalar( "PC Damage Cooldown", ImGuiDataType_Float,
                         pc_damage_cooldown.get_value_ptr(), pc_damage_cooldown.get_min_value_ptr(),
                         pc_damage_cooldown.get_max_value_ptr(), "%.1f seconds" );

    // Bomb Settings
    ImGui::SeparatorText( "Bomb Settings" );

    auto &bomb_damage = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::BombDamage>(
        getReg() );
    ImGui::SliderScalar( "Bomb Damage", ImGuiDataType_U8, bomb_damage.get_value_ptr(),
                         bomb_damage.get_min_value_ptr(), bomb_damage.get_max_value_ptr(), "%d" );

    auto &fuse_delay = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::FuseDelay>(
        getReg() );
    ImGui::SliderScalar( "Fuse Delay", ImGuiDataType_Float, fuse_delay.get_value_ptr(),
                         fuse_delay.get_min_value_ptr(), fuse_delay.get_max_value_ptr(),
                         "%.1f seconds" );

    auto &armed_on_delay = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::ArmedOnDelay>(
        getReg() );
    ImGui::SliderScalar( "Armed Detonation Delay Increment", ImGuiDataType_Float,
                         armed_on_delay.get_value_ptr(), armed_on_delay.get_min_value_ptr(),
                         armed_on_delay.get_max_value_ptr(), "%.3f" );

    auto &armed_off_delay = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::ArmedOffDelay>(
        getReg() );
    ImGui::SliderScalar( "Armed Off Delay", ImGuiDataType_Float, armed_off_delay.get_value_ptr(),
                         armed_off_delay.get_min_value_ptr(), armed_off_delay.get_max_value_ptr(),
                         "%.3f" );

    // Hazard Settings
    ImGui::SeparatorText( "Hazard Settings" );
    auto &corruption_damage = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::CorruptionDamage>( getReg() );
    ImGui::SliderScalar( "Corruption Damage", ImGuiDataType_U8, corruption_damage.get_value_ptr(),
                         corruption_damage.get_min_value_ptr(),
                         corruption_damage.get_max_value_ptr(), "%d" );

    auto &wormhole_anim_framerate = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::WormholeAnimFramerate>( getReg() );
    ImGui::SliderScalar( "Wormhole Animation Framerate", ImGuiDataType_Float,
                         wormhole_anim_framerate.get_value_ptr(),
                         wormhole_anim_framerate.get_min_value_ptr(),
                         wormhole_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &wormhole_seed = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::WormholeSeed>(
        getReg() );
    ImGui::InputScalar( "Wormhole Seed. Zero is ignored", ImGuiDataType_U64,
                        wormhole_seed.get_value_ptr(), nullptr, nullptr, "%llu",
                        ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    auto &corruption_seed = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::CorruptionSeed>(
        getReg() );
    ImGui::InputScalar( "Corruption Seed. Zero is ignored", ImGuiDataType_U64,
                        corruption_seed.get_value_ptr(), nullptr, nullptr, "%llu",
                        ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    auto &sinkhole_seed = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::SinkholeSeed>(
        getReg() );
    ImGui::InputScalar( "Sinkhole Seed. Zero is ignored", ImGuiDataType_U64,
                        sinkhole_seed.get_value_ptr(), nullptr, nullptr, "%llu",
                        ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_AutoSelectAll );

    // Loot Settings
    ImGui::SeparatorText( "Loot Settings" );
    auto &health_bonus = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::HealthBonus>(
        getReg() );
    ImGui::SliderScalar( "Health Bonus", ImGuiDataType_U8, health_bonus.get_value_ptr(),
                         health_bonus.get_min_value_ptr(), health_bonus.get_max_value_ptr(), "%d" );

    auto &bomb_bonus = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::BombBonus>(
        getReg() );
    ImGui::SliderScalar( "Bomb Bonus", ImGuiDataType_U8, bomb_bonus.get_value_ptr(),
                         bomb_bonus.get_min_value_ptr(), bomb_bonus.get_max_value_ptr(), "%d" );

    // NPC Settings
    ImGui::SeparatorText( "NPC Settings" );

    auto &npc_anim_framerate = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::NpcSkeleAnimFramerate>( getReg() );
    ImGui::SliderScalar( "NPC Animation Skeleton Framerate", ImGuiDataType_Float,
                         npc_anim_framerate.get_value_ptr(), npc_anim_framerate.get_min_value_ptr(),
                         npc_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_ghost_anim_framerate = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::NpcGhostAnimFramerate>( getReg() );
    ImGui::SliderScalar( "NPC Animation Ghost Framerate", ImGuiDataType_Float,
                         npc_ghost_anim_framerate.get_value_ptr(),
                         npc_ghost_anim_framerate.get_min_value_ptr(),
                         npc_ghost_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_damage = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::NpcDamage>(
        getReg() );
    ImGui::SliderScalar( "NPC Damage", ImGuiDataType_U8, npc_damage.get_value_ptr(),
                         npc_damage.get_min_value_ptr(), npc_damage.get_max_value_ptr(), "%d" );

    auto &npc_push_back = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::NpcPushBack>(
        getReg() );
    if ( ImGui::SliderScalar( "NPC Push Back Distance", ImGuiDataType_Float,
                              npc_push_back.get_value_ptr(), npc_push_back.get_min_value_ptr(),
                              npc_push_back.get_max_value_ptr(), "%.1f pixels" ) )
    {
      float step = Constants::kGridSquareSizePixelsF.x;
      npc_push_back.get_value() = roundf( npc_push_back.get_value() / step ) * step;
    }

    auto &npc_death_anim_framerate = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::NpcDeathAnimFramerate>( getReg() );
    ImGui::SliderScalar( "NPC Death Animation Framerate", ImGuiDataType_Float,
                         npc_death_anim_framerate.get_value_ptr(),
                         npc_death_anim_framerate.get_min_value_ptr(),
                         npc_death_anim_framerate.get_max_value_ptr(), "%.2f" );

    auto &npc_activate_scale = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::NpcActivateScale>( getReg() );
    ImGui::SliderScalar( "NPC Activation Bounding Box Scale Factor", ImGuiDataType_Float,
                         npc_activate_scale.get_value_ptr(), npc_activate_scale.get_min_value_ptr(),
                         npc_activate_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &npc_scan_scale = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::NpcScanScale>(
        getReg() );
    ImGui::SliderScalar( "NPC Scan Bounding Box Scale Factor", ImGuiDataType_Float,
                         npc_scan_scale.get_value_ptr(), npc_scan_scale.get_min_value_ptr(),
                         npc_scan_scale.get_max_value_ptr(), "%.1f pixels" );

    auto &npc_lerp_speed = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::NpcLerpSpeed>(
        getReg() );
    ImGui::SliderScalar( "NPC Speed", ImGuiDataType_Float, npc_lerp_speed.get_value_ptr(),
                         npc_lerp_speed.get_min_value_ptr(), npc_lerp_speed.get_max_value_ptr(),
                         "%.1f" );

    // Audio
    ImGui::SeparatorText( "Audio" );

    auto &music_volume = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::MusicVolume>(
        getReg() );
    ImGui::SliderScalar( "Music Volume", ImGuiDataType_Float, music_volume.get_value_ptr(),
                         music_volume.get_min_value_ptr(), music_volume.get_max_value_ptr(),
                         "%.1f" );

    auto &effects_volume = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::EffectsVolume>(
        getReg() );
    ImGui::SliderScalar( "Effects Volume", ImGuiDataType_Float, effects_volume.get_value_ptr(),
                         effects_volume.get_min_value_ptr(), effects_volume.get_max_value_ptr(),
                         "%.1f" );

    // Procedural Generation
    ImGui::SeparatorText( "Procedural Generation" );

    auto &max_num_altars = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::MaxNumAltars>(
        getReg() );
    ImGui::SliderScalar( "Max NNumber of Altars", ImGuiDataType_U8, max_num_altars.get_value_ptr(),
                         max_num_altars.get_min_value_ptr(), max_num_altars.get_max_value_ptr(),
                         "%d Max Altars" );

    auto &grave_num_multiplier = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::GraveNumMultiplier>( getReg() );
    ImGui::SliderScalar(
        "Grave Number Multiplier", ImGuiDataType_U8, grave_num_multiplier.get_value_ptr(),
        grave_num_multiplier.get_min_value_ptr(), grave_num_multiplier.get_max_value_ptr(), "%d" );

    auto &exit_key_requirement = Sys::PersistentSystem::get_persist_cmp<
        Cmp::Persistent::ExitKeyRequirement>( getReg() );
    ImGui::SliderScalar( "Exit Key Requirement", ImGuiDataType_U8,
                         exit_key_requirement.get_value_ptr(),
                         exit_key_requirement.get_min_value_ptr(),
                         exit_key_requirement.get_max_value_ptr(), "%d Keys" );

    auto &max_num_crypts = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::MaxNumCrypts>(
        getReg() );
    ImGui::SliderScalar( "Max Number of Crypts", ImGuiDataType_U8, max_num_crypts.get_value_ptr(),
                         max_num_crypts.get_min_value_ptr(), max_num_crypts.get_max_value_ptr(),
                         "%d Max Crypts" );
  } catch ( const std::exception &e )
  {
    ImGui::TextColored( ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ), "Error rendering settings: %s",
                        e.what() );
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

  auto &music_volume = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::MusicVolume>(
      getReg() );
  ImGui::SliderFloat( "Music Volume", &music_volume.get_value(), 0.f, 100.f, "%.1f" );

  auto &effects_volume = Sys::PersistentSystem::get_persist_cmp<Cmp::Persistent::EffectsVolume>(
      getReg() );
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

    sf::Text title_text( m_font, "You died...again!", 96 );
    title_text.setFillColor( sf::Color::White );
    title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
    m_window.draw( title_text );

    sf::Text start_text( m_font, "Press <R> key to continue", 48 );
    start_text.setFillColor( sf::Color::White );
    start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
    m_window.draw( start_text );

    auto key_view = getReg().view<Cmp::PlayerKeysCount>();
    for ( auto [entity, keycount] : key_view.each() )
    {
      sf::Text player_score_text( m_font, "KeyCount: " + std::to_string( keycount.get_count() ),
                                  24 );
      player_score_text.setFillColor( sf::Color::White );
      player_score_text.setPosition( { kDisplaySize.x / 4.f, 500.f } );
      m_window.draw( player_score_text );
    }

    auto candle_view = getReg().view<Cmp::PlayerCandlesCount>();
    for ( auto [entity, candlecount] : candle_view.each() )
    {
      sf::Text player_score_text( m_font,
                                  "CandleCount: " + std::to_string( candlecount.get_count() ), 24 );
      player_score_text.setFillColor( sf::Color::White );
      player_score_text.setPosition( { kDisplaySize.x / 4.f, 550.f } );
      m_window.draw( player_score_text );
    }

    auto relic_view = getReg().view<Cmp::PlayerRelicCount>();
    for ( auto [entity, reliccount] : relic_view.each() )
    {
      sf::Text player_score_text( m_font, "RelicCount: " + std::to_string( reliccount.get_count() ),
                                  24 );
      player_score_text.setFillColor( sf::Color::White );
      player_score_text.setPosition( { kDisplaySize.x / 4.f, 600.f } );
      m_window.draw( player_score_text );
    }
  }

  m_window.display();
  // main render end
}

void RenderMenuSystem::render_victory_screen()
{
  // main render begin
  m_window.clear();

  sf::Text title_text( m_font, "You won!", 96 );
  title_text.setFillColor( sf::Color::White );
  title_text.setPosition( { kDisplaySize.x / 4.f, 100.f } );
  m_window.draw( title_text );

  sf::Text start_text( m_font, "Press <R> key to continue", 48 );
  start_text.setFillColor( sf::Color::White );
  start_text.setPosition( { kDisplaySize.x / 4.f, 200.f } );
  m_window.draw( start_text );

  auto key_view = getReg().view<Cmp::PlayerKeysCount>();
  for ( auto [entity, keycount] : key_view.each() )
  {
    sf::Text player_score_text( m_font, "KeyCount: " + std::to_string( keycount.get_count() ), 24 );
    player_score_text.setFillColor( sf::Color::White );
    player_score_text.setPosition( { kDisplaySize.x / 4.f, 500.f } );
    m_window.draw( player_score_text );
  }

  auto candle_view = getReg().view<Cmp::PlayerCandlesCount>();
  for ( auto [entity, candlecount] : candle_view.each() )
  {
    sf::Text player_score_text( m_font, "CandleCount: " + std::to_string( candlecount.get_count() ),
                                24 );
    player_score_text.setFillColor( sf::Color::White );
    player_score_text.setPosition( { kDisplaySize.x / 4.f, 550.f } );
    m_window.draw( player_score_text );
  }

  auto relic_view = getReg().view<Cmp::PlayerRelicCount>();
  for ( auto [entity, reliccount] : relic_view.each() )
  {
    sf::Text player_score_text( m_font, "RelicCount: " + std::to_string( reliccount.get_count() ),
                                24 );
    player_score_text.setFillColor( sf::Color::White );
    player_score_text.setPosition( { kDisplaySize.x / 4.f, 600.f } );
    m_window.draw( player_score_text );
  }

  m_window.display();
  // main render end
}

} // namespace ProceduralMaze::Sys