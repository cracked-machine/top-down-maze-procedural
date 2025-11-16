#ifndef SCENE_CRYPTSCENE_HPP_
#define SCENE_CRYPTSCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class CryptScene : public IScene
{
public:
  CryptScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] sf::Time dt ) override;

  entt::registry *get_registry() override;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_CRYPTSCENE_HPP_