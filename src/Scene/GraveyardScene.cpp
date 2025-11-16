#include <Scene/GraveyardScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene() {}

void GraveyardScene::on_enter() { /* init entities */ }
void GraveyardScene::on_exit() { /* cleanup if needed */ }
void GraveyardScene::update( [[maybe_unused]] sf::Time dt )
{
  //     m_sound_bank->get_music( "title_music" ).stop();
  //     if ( m_sound_bank->get_music( "game_music" ).getStatus() != sf::Music::Status::Playing )
  //     {
  //       m_sound_bank->get_music( "game_music" ).play();
  //     }

  //     // play/stop footstep sounds depending on player movement
  //     auto player_view = m_reg->view<Cmp::PlayableCharacter, Cmp::Direction>();
  //     for ( auto [pc_entity, pc_cmp, dir_cmp] : player_view.each() )
  //     {
  //       if ( dir_cmp == sf::Vector2f( 0.f, 0.f ) ) { m_player_sys->stop_footsteps_sound(); }
  //       else { m_player_sys->play_footsteps_sound(); }
  //     }

  //     m_event_handler->game_state_handler();

  //     // m_flood_sys.update();
  //     m_anim_sys->update( globalDeltaTime );
  //     m_sinkhole_sys->update_hazard_field();
  //     m_corruption_sys->update_hazard_field();
  //     m_bomb_sys->update();

  //     m_sinkhole_sys->check_npc_hazard_field_collision();
  //     m_corruption_sys->check_npc_hazard_field_collision();
  //     m_exit_sys->check_exit_collision();
  //     m_loot_sys->check_loot_collision();
  //     m_npc_sys->check_bones_reanimation();
  //     m_wormhole_sys->check_player_wormhole_collision();
  //     m_digging_sys->update();
  //     m_footstep_sys->update();

  //     // Throttled obstacle distance update (optimization)
  //     if ( m_obstacle_distance_timer.getElapsedTime() >= m_obstacle_distance_update_interval )
  //     {
  //       m_path_find_sys->update_player_distances();
  //       m_obstacle_distance_timer.restart();
  //     }

  //     // enable/disable collision detection depending on Cmp::System settings
  //     for ( auto [_ent, _sys] : m_reg->view<Cmp::System>().each() )
  //     {
  //       m_player_sys->update_movement( globalDeltaTime, !_sys.collisions_enabled );
  //       if ( _sys.collisions_enabled )
  //       {
  //         m_sinkhole_sys->check_player_hazard_field_collision();
  //         m_corruption_sys->check_player_hazard_field_collision();
  //         m_npc_sys->check_player_to_npc_collision();
  //       }
  //       if ( _sys.level_complete )
  //       {
  //         SPDLOG_INFO( "Level complete!" );
  //         game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER;
  //       }
  //     }

  //     auto player_entity = m_reg->view<Cmp::PlayableCharacter>().front();
  //     m_path_find_sys->findPath( player_entity );
  //     m_npc_sys->update_movement( globalDeltaTime );

  //     // did the player die? Then end the game
  //     if ( m_player_sys->check_player_mortality() == Cmp::PlayerMortality::State::DEAD )
  //     {
  //       SPDLOG_INFO( "Player has died!" );
  //       game_state.current_state = Cmp::Persistent::GameState::State::GAMEOVER;
  //     }

  //     m_render_game_sys->render_game( globalDeltaTime, *m_render_overlay_sys, *m_render_player_sys );
}

entt::registry *GraveyardScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene