#include <Scene/LevelCompleteScene.hpp>
#include <Scene/SceneManager.hpp>
#include <SystemStore.hpp>

namespace ProceduralMaze::Scene
{

LevelCompleteScene::LevelCompleteScene( Audio::SoundBank &sound_bank, Sys::SystemStore &system_store )
    : m_sound_bank( sound_bank ),
      m_system_store( system_store )
{
}

void LevelCompleteScene::on_init() { /* init entities */ }
void LevelCompleteScene::on_enter()
{
  SPDLOG_INFO( "Entering LevelCompleteScene" );
  auto &persistent_sys = static_cast<Sys::PersistentSystem &>( m_system_store.find( Sys::SystemStore::Type::PersistentSystem ) );
  persistent_sys.initializeComponentRegistry();
  persistent_sys.load_state();
  m_sound_bank.get_music( "game_music" ).stop();
  m_sound_bank.get_music( "title_music" ).play();
}
void LevelCompleteScene::on_exit()
{
  SPDLOG_INFO( "Exiting LevelCompleteScene" );
  registry.clear();

  auto &player_sys = static_cast<Sys::PlayerSystem &>( m_system_store.find( Sys::SystemStore::Type::PlayerSystem ) );
  player_sys.stop_footsteps_sound();

  auto &m_render_game_sys = static_cast<Sys::RenderGameSystem &>( m_system_store.find( Sys::SystemStore::Type::RenderGameSystem ) );
  m_render_game_sys.clear_tilemap();
}
void LevelCompleteScene::update( [[maybe_unused]] sf::Time dt )
{
  m_sound_bank.get_effect( "footsteps" ).stop();

  auto &render_menu_sys = static_cast<Sys::RenderMenuSystem &>( m_system_store.find( Sys::SystemStore::Type::RenderMenuSystem ) );
  render_menu_sys.render_victory_screen();

  auto &m_event_handler = static_cast<Sys::EventHandler &>( m_system_store.find( Sys::SystemStore::Type::EventHandler ) );
  auto menu_action = m_event_handler.game_over_state_handler();
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