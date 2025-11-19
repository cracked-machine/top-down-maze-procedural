#include <Scene/LevelCompleteScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

LevelCompleteScene::LevelCompleteScene( Audio::SoundBank &sound_bank, Sys::PersistentSystem *persistent_sys,
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

void LevelCompleteScene::on_init() { /* init entities */ }
void LevelCompleteScene::on_enter()
{
  SPDLOG_INFO( "Entering LevelCompleteScene" );
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}
void LevelCompleteScene::on_exit()
{
  SPDLOG_INFO( "Exiting LevelCompleteScene" );
  registry.clear();

  m_player_sys->stop_footsteps_sound();
  m_render_game_sys->clear_tilemap();
}
void LevelCompleteScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  m_render_menu_sys->render_victory_screen();

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

entt::registry *LevelCompleteScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene