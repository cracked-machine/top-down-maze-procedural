#include <Scene/GameOverScene.hpp>
#include <Scene/SceneManager.hpp>
#include <Systems/PersistentSystem.hpp>

namespace ProceduralMaze::Scene
{

GameOverScene::GameOverScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
                              Sys::PlayerSystem *player_sys, Sys::RenderMenuSystem *render_menu_sys,
                              Sys::EventHandler *event_handler, Sys::RenderGameSystem *render_game_sys )
    : m_sound_bank( sound_bank ),
      m_persistent_sys( persistent_sys ),
      m_player_sys( player_sys ),
      m_render_menu_sys( render_menu_sys ),
      m_event_handler( event_handler ),
      m_render_game_sys( render_game_sys )
{
}

void GameOverScene::on_init() { SPDLOG_INFO( "Initializing GameOverScene" ); }
void GameOverScene::on_enter()
{
  SPDLOG_INFO( "Entering GameOverScene" );
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}
void GameOverScene::on_exit()
{
  SPDLOG_INFO( "Exiting GameOverScene" );
  registry.clear();

  m_player_sys->stop_footsteps_sound();
  m_render_game_sys->clear_tilemap();
}
void GameOverScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  m_render_menu_sys->render_defeat_screen();

  auto menu_action = m_event_handler->game_over_state_handler();
  switch ( menu_action )
  {
    case Sys::EventHandler::NavigationActions::TITLE:
      request( SceneRequest::Pop );
      break;
    default:
      break;
  }
}

entt::registry *GameOverScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene