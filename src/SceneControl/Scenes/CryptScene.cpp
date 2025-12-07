#include <SceneControl/Scenes/CryptScene.hpp>

namespace ProceduralMaze::Scene
{

void CryptScene::on_init()
{ // initialize any entities or components that should exist before on_enter
  auto &m_persistent_sys = m_system_store.find<Sys::SystemStore::Type::PersistentSystem>();
  m_persistent_sys.initializeComponentRegistry();
  m_persistent_sys.load_state();
}

void CryptScene::on_enter()
{
  // Initialize entities specific to the CryptScene
  SPDLOG_INFO( "Entering {}", get_name() );
}

void CryptScene::on_exit()
{
  // Cleanup any resources or entities specific to the CryptScene
  SPDLOG_INFO( "Exiting {}", get_name() );
}

void CryptScene::do_update( [[maybe_unused]] sf::Time dt )
{
  // Update game logic specific to the CryptScene
}

entt::registry &CryptScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene