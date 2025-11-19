#include <Scene/GraveyardScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/BombSystem.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene(
    Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys, Sys::PlayerSystem *player_sys,
    Sys::RenderGameSystem *render_game_sys, Sys::EventHandler *event_handler, Sys::AnimSystem *anim_sys,
    Sys::SinkHoleHazardSystem *sinkhole_sys, Sys::CorruptionHazardSystem *corruption_sys, Sys::BombSystem *bomb_sys,
    Sys::ExitSystem *exit_sys, Sys::LootSystem *loot_sys, Sys::NpcSystem *npc_sys, Sys::WormholeSystem *wormhole_sys,
    Sys::DiggingSystem *digging_sys, Sys::FootstepSystem *footstep_sys, Sys::PathFindSystem *path_find_sys,
    Sys::RenderOverlaySystem *render_overlay_sys, Sys::RenderPlayerSystem *render_player_sys,
    Sys::ProcGen::RandomLevelGenerator *random_level_sys, Sys::ProcGen::CellAutomataSystem *cellauto_parser )

    : m_sound_bank( sound_bank ),
      m_persistent_sys( persistent_sys ),
      m_player_sys( player_sys ),
      m_event_handler( event_handler ),
      m_render_game_sys( render_game_sys ),
      m_anim_sys( anim_sys ),
      m_sinkhole_sys( sinkhole_sys ),
      m_corruption_sys( corruption_sys ),
      m_bomb_sys( bomb_sys ),
      m_exit_sys( exit_sys ),
      m_loot_sys( loot_sys ),
      m_npc_sys( npc_sys ),
      m_wormhole_sys( wormhole_sys ),
      m_digging_sys( digging_sys ),
      m_footstep_sys( footstep_sys ),
      m_path_find_sys( path_find_sys ),
      m_render_overlay_sys( render_overlay_sys ),
      m_render_player_sys( render_player_sys ),
      random_level_sys( random_level_sys ),
      cellauto_parser( cellauto_parser )
{
}

void GraveyardScene::on_init()
{
  // initialize any entities or components that should exist before on_enter
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
}

void GraveyardScene::on_enter()
{
  SPDLOG_INFO( "Entering GraveyardScene" );

  m_render_game_sys->init_shaders();
  m_render_game_sys->init_tilemap();

  m_player_sys->add_player_entity();

  auto entity = registry.create();
  registry.emplace<Cmp::System>( entity );

  random_level_sys->generate();
  cellauto_parser->set_random_level_generator( random_level_sys );
  cellauto_parser->iterate( 5 );

  m_exit_sys->spawn_exit();
  m_render_game_sys->init_views();
  m_sinkhole_sys->init_hazard_field();
  m_corruption_sys->init_hazard_field();
  m_wormhole_sys->spawn_wormhole( Sys::WormholeSystem::SpawnPhase::InitialSpawn );
}

void GraveyardScene::on_exit()
{
  SPDLOG_INFO( "Exiting GraveyardScene" );
  registry.clear();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
  m_player_sys->stop_footsteps_sound();
  m_render_game_sys->clear_tilemap();
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
  auto player_view = registry.view<Cmp::PlayableCharacter, Cmp::Direction>();
  for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
  {
    if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { m_player_sys->stop_footsteps_sound(); }
    else { m_player_sys->play_footsteps_sound(); }
  }

  auto menu_action = m_event_handler->game_state_handler();
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

  m_anim_sys->update( dt );
  m_sinkhole_sys->update_hazard_field();
  m_corruption_sys->update_hazard_field();
  m_bomb_sys->update();

  m_sinkhole_sys->check_npc_hazard_field_collision();
  m_corruption_sys->check_npc_hazard_field_collision();
  m_exit_sys->check_exit_collision();
  m_loot_sys->check_loot_collision();
  m_npc_sys->check_bones_reanimation();
  m_wormhole_sys->check_player_wormhole_collision();
  m_digging_sys->update();
  m_footstep_sys->update();

  // Throttled obstacle distance update (optimization)
  if ( m_obstacle_distance_timer.getElapsedTime() >= m_obstacle_distance_update_interval )
  {
    m_path_find_sys->update_player_distances();
    m_obstacle_distance_timer.restart();
  }

  // enable/disable collision detection depending on Cmp::System settings
  for ( auto [_ent, _sys] : registry.view<Cmp::System>().each() )
  {
    m_player_sys->update_movement( dt, !_sys.collisions_enabled );
    if ( _sys.collisions_enabled )
    {
      m_sinkhole_sys->check_player_hazard_field_collision();
      m_corruption_sys->check_player_hazard_field_collision();
      m_npc_sys->check_player_to_npc_collision();
    }
    if ( _sys.level_complete )
    {
      SPDLOG_INFO( "Level complete!" );
      request( SceneRequest::LevelComplete );
    }
  }

  auto player_entity = registry.view<Cmp::PlayableCharacter>().front();
  m_path_find_sys->findPath( player_entity );
  m_npc_sys->update_movement( dt );

  // did the player die? Then end the game
  if ( m_player_sys->check_player_mortality() == Cmp::PlayerMortality::State::DEAD )
  {
    SPDLOG_INFO( "Player has died!" );
    request( SceneRequest::GameOver );
  }

  m_render_game_sys->render_game( dt, *m_render_overlay_sys, *m_render_player_sys );
}

entt::registry *GraveyardScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene