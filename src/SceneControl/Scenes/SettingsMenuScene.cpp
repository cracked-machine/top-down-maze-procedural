#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <SceneControl/Scenes/SettingsMenuScene.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

SettingsMenuScene::SettingsMenuScene( Sys::SystemStore &system_store, entt::dispatcher &nav_event_dispatcher )
    : m_system_store( system_store ),
      m_nav_event_dispatcher( nav_event_dispatcher )
{
}

void SettingsMenuScene::on_init() { SPDLOG_INFO( "Initializing SettingsMenuScene" ); }

void SettingsMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering  {}", get_name() );
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}
void SettingsMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting  {}", get_name() );
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.save_state();
}
void SettingsMenuScene::update( [[maybe_unused]] sf::Time dt )
{
  auto &m_render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  m_render_menu_sys.render_settings( dt );

  // Notify SceneInputRouter that there may be new SettingsMenuScene input to process
  m_nav_event_dispatcher.enqueue( Events::ProcessSettingsMenuSceneInputEvent() );
}

entt::registry &SettingsMenuScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene