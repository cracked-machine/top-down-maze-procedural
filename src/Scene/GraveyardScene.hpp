#ifndef SCENE_GRAVEYARDSCENE_HPP_
#define SCENE_GRAVEYARDSCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class GraveyardScene : public IScene
{
public:
  GraveyardScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;

  entt::registry *get_registry() override;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GRAVEYARDSCENE_HPP_