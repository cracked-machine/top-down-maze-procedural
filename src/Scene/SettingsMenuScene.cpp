#include <Scene/SettingsMenuScene.hpp>

namespace ProceduralMaze::Scene
{

SettingsMenuScene::SettingsMenuScene()
    : reg{}
{
}

void SettingsMenuScene::on_enter() { /* init entities */ }
void SettingsMenuScene::on_exit() { /* cleanup if needed */ }
void SettingsMenuScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *SettingsMenuScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene