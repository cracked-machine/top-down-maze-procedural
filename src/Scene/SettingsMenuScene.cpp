#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>

namespace ProceduralMaze::Scene
{

SettingsMenuScene::SettingsMenuScene( Sys::PersistentSystem *persistent_sys, Sys::RenderMenuSystem *render_menu_sys,
                                      Sys::EventHandler *event_handler )
    : m_persistent_sys( persistent_sys ),
      m_render_menu_sys( render_menu_sys ),
      m_event_handler( event_handler )
{
}

void SettingsMenuScene::on_init() { SPDLOG_INFO( "Initializing SettingsMenuScene" ); }

void SettingsMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering SettingsMenuScene" );
  m_persistent_sys->initializeComponentRegistry();
  m_persistent_sys->load_state();
}
void SettingsMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting SettingsMenuScene" );
  m_persistent_sys->save_state();
}
void SettingsMenuScene::update( [[maybe_unused]] sf::Time dt )
{

  m_render_menu_sys->render_settings( dt );
  auto menu_action = m_event_handler->settings_state_handler();
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