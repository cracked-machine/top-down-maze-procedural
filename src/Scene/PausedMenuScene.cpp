#include <Scene/PausedMenuScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene() {}

void PausedMenuScene::on_enter() { /* init entities */ }
void PausedMenuScene::on_exit() { /* cleanup if needed */ }
void PausedMenuScene::update( [[maybe_unused]] sf::Time dt ) { /* game logic */ }

entt::registry *PausedMenuScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene