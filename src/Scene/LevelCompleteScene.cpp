#include <Scene/LevelCompleteScene.hpp>

namespace ProceduralMaze::Scene
{

LevelCompleteScene::LevelCompleteScene()
    : reg{}
{
}

void LevelCompleteScene::on_enter() { /* init entities */ }
void LevelCompleteScene::on_exit() { /* cleanup if needed */ }
void LevelCompleteScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *LevelCompleteScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene