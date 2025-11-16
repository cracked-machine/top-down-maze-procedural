#include <Scene/LevelCompleteScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

LevelCompleteScene::LevelCompleteScene() {}

void LevelCompleteScene::on_enter() { /* init entities */ }
void LevelCompleteScene::on_exit() { /* cleanup if needed */ }
void LevelCompleteScene::update( [[maybe_unused]] sf::Time dt ) { /* game logic */ }

entt::registry *LevelCompleteScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene