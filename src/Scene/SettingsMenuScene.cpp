#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>
#include <SystemStore.hpp>

namespace ProceduralMaze::Scene
{

SettingsMenuScene::SettingsMenuScene( Sys::SystemStore &system_store )
    : m_system_store( system_store )
{
}

void SettingsMenuScene::on_init() { SPDLOG_INFO( "Initializing SettingsMenuScene" ); }

void SettingsMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering SettingsMenuScene" );
  auto &m_persistent_sys = static_cast<Sys::PersistentSystem &>( m_system_store.find( Sys::SystemStore::Type::PersistentSystem ) );
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}
void SettingsMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting SettingsMenuScene" );
  auto &m_persistent_sys = static_cast<Sys::PersistentSystem &>( m_system_store.find( Sys::SystemStore::Type::PersistentSystem ) );
  m_persistent_sys.save_state();
}
void SettingsMenuScene::update( [[maybe_unused]] sf::Time dt )
{
  auto &m_render_menu_sys = static_cast<Sys::RenderMenuSystem &>( m_system_store.find( Sys::SystemStore::Type::RenderMenuSystem ) );
  m_render_menu_sys.render_settings( dt );

  auto &m_event_handler = static_cast<Sys::EventHandler &>( m_system_store.find( Sys::SystemStore::Type::EventHandler ) );
  auto menu_action = m_event_handler.settings_state_handler();
  switch ( menu_action )
  {
    case Sys::EventHandler::NavigationActions::TITLE:
      request( SceneRequest::Pop );
      break;
    default:
      break;
  }
}

entt::registry *SettingsMenuScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene