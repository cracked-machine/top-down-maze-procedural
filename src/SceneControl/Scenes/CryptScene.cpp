#include <SceneControl/Scenes/CryptScene.hpp>

namespace ProceduralMaze::Scene
{

CryptScene::CryptScene() {}

void CryptScene::on_init() { /* init entities */ }
void CryptScene::on_enter() { /* init entities */ }
void CryptScene::on_exit() { /* cleanup if needed */ }
void CryptScene::update( [[maybe_unused]] sf::Time dt ) { /* game logic */ }

entt::registry &CryptScene::get_registry() { return m_reg; }

} // namespace ProceduralMaze::Scene