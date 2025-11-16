#ifndef SCENE_GAMEOVERSCENE_HPP_
#define SCENE_GAMEOVERSCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class GameOverScene : public IScene
{
public:
  GameOverScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] float dt ) override;

  entt::registry *get_registry() override;

private:
  entt::registry reg;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_GAMEOVERSCENE_HPP_