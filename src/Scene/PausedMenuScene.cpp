#include <Scene/PausedMenuScene.hpp>

namespace ProceduralMaze::Scene
{

PausedMenuScene::PausedMenuScene()
    : reg{}
{
}

void PausedMenuScene::on_enter() { /* init entities */ }
void PausedMenuScene::on_exit() { /* cleanup if needed */ }
void PausedMenuScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *PausedMenuScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene