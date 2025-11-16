#include <Scene/GameOverScene.hpp>
#include <Scene/SceneManager.hpp>

namespace ProceduralMaze::Scene
{

GameOverScene::GameOverScene() {}

void GameOverScene::on_enter() { /* init entities */ }
void GameOverScene::on_exit() { /* cleanup if needed */ }
void GameOverScene::update( [[maybe_unused]] sf::Time dt ) { /* game logic */ }

entt::registry *GameOverScene::get_registry() { return &registry; }

} // namespace ProceduralMaze::Scene