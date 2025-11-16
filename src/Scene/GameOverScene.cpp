#include <Scene/GameOverScene.hpp>

namespace ProceduralMaze::Scene
{

GameOverScene::GameOverScene()
    : reg{}
{
}

void GameOverScene::on_enter() { /* init entities */ }
void GameOverScene::on_exit() { /* cleanup if needed */ }
void GameOverScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *GameOverScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene