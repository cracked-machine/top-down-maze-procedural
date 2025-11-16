#ifndef SCENE_PAUSEDMENUSCENE_HPP_
#define SCENE_PAUSEDMENUSCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class PausedMenuScene : public IScene
{
public:
  PausedMenuScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] float dt ) override;

  entt::registry *get_registry() override;

private:
  entt::registry reg;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_PAUSEDMENUSCENE_HPP_