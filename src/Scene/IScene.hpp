#ifndef SCENE_ISCENE_HPP_
#define SCENE_ISCENE_HPP_

#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Scene
{

class IScene
{
public:
  virtual ~IScene() = default;

  virtual void on_enter() = 0;
  virtual void on_exit() = 0;
  virtual void update( float dt ) = 0;

  virtual entt::registry *get_registry() = 0;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_ISCENE_HPP_