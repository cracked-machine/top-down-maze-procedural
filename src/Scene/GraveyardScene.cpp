#include <Scene/GraveyardScene.hpp>
#include <Scene/SceneManager.hpp>
#include <SystemStore.hpp>
#include <Systems/Render/RenderGameSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store )

    : m_sound_bank( sound_bank ),
      m_system_store( system_store )
{
}

void GraveyardScene::on_init()
{
  // initialize any entities or components that should exist before on_enter
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering GraveyardScene" );

  auto &render_game_system = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  SPDLOG_INFO( "Got render_game_system at {}", static_cast<void *>( &render_game_system ) );
  render_game_system.init_shaders();
  render_game_system.init_tilemap();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.add_player_entity();

  auto entity = m_reg.create();
  m_reg.emplace<Cmp::System>( entity );

  auto &random_level_sys = m_system_store.find<Sys::SystemStore::Type::RandomLevelGenerator>();
  random_level_sys.generate();

  auto &cellauto_parser = m_system_store.find<Sys::SystemStore::Type::CellAutomataSystem>();
  cellauto_parser.set_random_level_generator( &random_level_sys );
  cellauto_parser.iterate( 5 );

  auto &exit_sys = m_system_store.find<Sys::SystemStore::Type::ExitSystem>();
  exit_sys.spawn_exit();
  render_game_system.init_views();

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.init_hazard_field();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.init_hazard_field();

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting GraveyardScene" );
  m_reg.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.stop_footsteps_sound();

  auto &m_render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  m_render_game_sys.clear_tilemap();
}

void GraveyardScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_music( "title_music" ).stop();
  // only do this once every update, other it constantly restarts the music
  if ( m_sound_bank.get_music( "game_music" ).getStatus() != sf::Music::Status::Playing )
  {
    m_sound_bank.get_music( "game_music" ).play();
  }

  // play/stop footstep sounds depending on player movement
  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  auto player_view = m_reg.view<Cmp::PlayableCharacter, Cmp::Direction>();
  for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { m_player_sys.stop_footsteps_sound(); }
    else { m_player_sys.play_footsteps_sound(); }
  }

  auto &m_event_handler = m_system_store.find<Sys::SystemStore::Type::EventHandler>();
  auto menu_action = m_event_handler.game_state_handler();
  switch ( menu_action )
  {
    case Sys::EventHandler::NavigationActions::TITLE:
      request( SceneRequest::Pop );
      break;
    case Sys::EventHandler::NavigationActions::PAUSE:
      request( SceneRequest::PausedMenu );
      break;
    default:
      break;
  }

  auto &anim_sys = m_system_store.find<Sys::SystemStore::Type::AnimSystem>();
  anim_sys.update( dt );

  auto &sinkhole_sys = m_system_store.find<Sys::SystemStore::Type::SinkHoleHazardSystem>();
  sinkhole_sys.update_hazard_field();

  auto &corruption_sys = m_system_store.find<Sys::SystemStore::Type::CorruptionHazardSystem>();
  corruption_sys.update_hazard_field();

  auto &bomb_sys = m_system_store.find<Sys::SystemStore::Type::BombSystem>();
  bomb_sys.update();

  sinkhole_sys.check_npc_hazard_field_collision();

  corruption_sys.check_npc_hazard_field_collision();

  auto &exit_sys = m_system_store.find<Sys::SystemStore::Type::ExitSystem>();
  exit_sys.check_exit_collision();

  auto &loot_sys = m_system_store.find<Sys::SystemStore::Type::LootSystem>();
  loot_sys.check_loot_collision();

  auto &npc_sys = m_system_store.find<Sys::SystemStore::Type::NpcSystem>();
  npc_sys.check_bones_reanimation();

  auto &wormhole_sys = m_system_store.find<Sys::SystemStore::Type::WormholeSystem>();
  wormhole_sys.check_player_wormhole_collision();

  auto &digging_sys = m_system_store.find<Sys::SystemStore::Type::DiggingSystem>();
  digging_sys.update();

  auto &footstep_sys = m_system_store.find<Sys::SystemStore::Type::FootstepSystem>();
  footstep_sys.update();

  // Throttled obstacle distance update (optimization)
  auto &path_find_sys = m_system_store.find<Sys::SystemStore::Type::PathFindSystem>();
  if ( m_obstacle_distance_timer.getElapsedTime() >= m_obstacle_distance_update_interval )
  {
    path_find_sys.update_player_distances();
    m_obstacle_distance_timer.restart();
  }

  // enable/disable collision detection depending on Cmp::System settings
  auto &player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  for ( auto [_ent, _sys] : m_reg.view<Cmp::System>().each() )
  {
    player_sys.update_movement( dt, !_sys.collisions_enabled );
    if ( _sys.collisions_enabled )
    {
      sinkhole_sys.check_player_hazard_field_collision();
      corruption_sys.check_player_hazard_field_collision();
      npc_sys.check_player_to_npc_collision();
    }
    if ( _sys.level_complete )
    {
      SPDLOG_INFO( "Level complete!" );
      request( SceneRequest::LevelComplete );
    }
  }

  auto player_entity = m_reg.view<Cmp::PlayableCharacter>().front();
  path_find_sys.findPath( player_entity );
  npc_sys.update_movement( dt );

  // did the player die? Then end the game
  if ( player_sys.check_player_mortality() == Cmp::PlayerMortality::State::DEAD )
  {
    SPDLOG_INFO( "Player has died!" );
    request( SceneRequest::GameOver );
  }

  auto &render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  auto &render_overlay_sys = m_system_store.find<Sys::SystemStore::Type::RenderOverlaySystem>();
  auto &render_player_sys = m_system_store.find<Sys::SystemStore::Type::RenderPlayerSystem>();
  render_game_sys.render_game( dt, render_overlay_sys, render_player_sys );
}

entt::registry &GraveyardScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene