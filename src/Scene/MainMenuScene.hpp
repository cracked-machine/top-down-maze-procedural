#ifndef SCENE_MAINMENUSCENE_HPP_
#define SCENE_MAINMENUSCENE_HPP_

#include <Scene/IScene.hpp>

namespace ProceduralMaze::Scene
{

class MainMenuScene : public IScene
{
public:
  MainMenuScene();

  void on_enter() override;
  void on_exit() override;
  void update( [[maybe_unused]] float dt ) override;

  entt::registry *get_registry() override;

private:
  entt::registry reg;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_MAINMENUSCENE_HPP_