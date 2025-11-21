#include <Scene/GameOverScene.hpp>
#include <Scene/SceneManager.hpp>
#include <SystemStore.hpp>
#include <Systems/PersistentSystem.hpp>

namespace ProceduralMaze::Scene
{

GameOverScene::GameOverScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store )
{
}

void GameOverScene::on_init() { SPDLOG_INFO( "Initializing GameOverScene" ); }
void GameOverScene::on_enter()
{
  SPDLOG_INFO( "Entering GameOverScene" );
  auto &persistent_sys = static_cast<Sys::PersistentSystem &>( m_system_store.find( Sys::SystemStore::Type::PersistentSystem ) );
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}
void GameOverScene::on_exit()
{
  SPDLOG_INFO( "Exiting GameOverScene" );
  registry.clear();

  auto &m_player_sys = m_system_store.find<Sys::SystemStore::Type::PlayerSystem>();
  m_player_sys.stop_footsteps_sound();

  auto &m_render_game_sys = m_system_store.find<Sys::SystemStore::Type::RenderGameSystem>();
  m_render_game_sys.clear_tilemap();
}
void GameOverScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  render_menu_sys.render_defeat_screen();

  auto &event_handler = m_system_store.find<Sys::SystemStore::Type::EventHandler>();
  auto menu_action = event_handler.game_over_state_handler();
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