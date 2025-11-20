#include <Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/SettingsMenuScene.hpp>

namespace ProceduralMaze::Scene
{

SettingsMenuScene::SettingsMenuScene( Sys::PersistentSystem *persistent_sys, Sys::RenderMenuSystem *render_menu_sys,
                                      entt::dispatcher &nav_event_dispatcher )
    : m_persistent_sys( persistent_sys ),
      m_render_menu_sys( render_menu_sys ),
      m_nav_event_dispatcher( nav_event_dispatcher )
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
  m_nav_event_dispatcher.trigger<Events::ProcessSettingsMenuSceneInputEvent>();
}

entt::registry *SettingsMenuScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene