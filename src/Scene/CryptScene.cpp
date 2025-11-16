#include <Scene/CryptScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

CryptScene::CryptScene() {}

void CryptScene::on_init() { /* init entities */ }
void CryptScene::on_enter() { /* init entities */ }
void CryptScene::on_exit() { /* cleanup if needed */ }
void CryptScene::update( [[maybe_unused]] sf::Time dt ) { /* game logic */ }

entt::registry *CryptScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene