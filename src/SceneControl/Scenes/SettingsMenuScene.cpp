#include <SceneControl/Scenes/SettingsMenuScene.hpp>

#include <Audio/SoundBank.hpp>
#include <SceneControl/Events/ProcessSettingsMenuSceneInputEvent.hpp>
#include <Systems/PersistSystemImpl.hpp>
#include <Systems/Render/RenderMenuSystem.hpp>
#include <Systems/SystemStore.hpp>

namespace ProceduralMaze::Scene
{

void SettingsMenuScene::on_init() { SPDLOG_INFO( "Initializing SettingsMenuScene" ); }

void SettingsMenuScene::on_enter()
{
  SPDLOG_INFO( "Entering  {}", get_name() );
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}
void SettingsMenuScene::on_exit()
{
  SPDLOG_INFO( "Exiting  {}", get_name() );
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistSystem>();
  m_persistent_sys.save_state();
}
void SettingsMenuScene::do_update( [[maybe_unused]] sf::Time dt )
{
  auto &m_render_menu_sys = m_system_store.find<Sys::SystemStore::Type::RenderMenuSystem>();
  m_render_menu_sys.render_settings( dt );
}

entt::registry &SettingsMenuScene::registry() { return m_reg; }

} // namespace ProceduralMaze::Scene