#ifndef SCENE_LEVELCOMPLETE_SCENE_HPP_
#define SCENE_LEVELCOMPLETE_SCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class LevelCompleteScene : public IScene
{
public:
  LevelCompleteScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] float dt ) override;

  entt::registry *get_registry() override;

private:
  entt::registry reg;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_LEVELCOMPLETE_SCENE_HPP_