#include <Scene/MainMenuScene.hpp>

namespace ProceduralMaze::Scene
{

MainMenuScene::MainMenuScene()
    : reg{}
{
}

void MainMenuScene::on_enter() { /* init entities */ }
void MainMenuScene::on_exit() { /* cleanup if needed */ }
void MainMenuScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *MainMenuScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene