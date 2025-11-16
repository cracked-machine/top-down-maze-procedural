#include <Scene/GraveyardScene.hpp>

namespace ProceduralMaze::Scene
{

GraveyardScene::GraveyardScene()
    : reg{}
{
}

void GraveyardScene::on_enter() { /* init entities */ }
void GraveyardScene::on_exit() { /* cleanup if needed */ }
void GraveyardScene::update( [[maybe_unused]] float dt ) { /* game logic */ }

entt::registry *GraveyardScene::get_registry() { return &reg; }

} // namespace ProceduralMaze::Scene