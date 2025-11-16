#include <Scene/CryptScene.hpp>

namespace ProceduralMaze::Scene
{

CryptScene::CryptScene()
    : reg{}
{
}

void CryptScene::on_enter() { /* init entities */ }
void CryptScene::on_exit() { /* cleanup if needed */ }
void CryptScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *CryptScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene