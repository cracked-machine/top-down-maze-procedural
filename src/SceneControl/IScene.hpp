#ifndef SCENE_ISCENE_HPP_
#define SCENE_ISCENE_HPP_

#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <string>

namespace ProceduralMaze::Scene
{

class IScene
{
public:
  virtual ~IScene() = default;

  virtual void on_init() = 0;
  virtual void on_enter() = 0;
  virtual void on_exit() = 0;
  virtual void update( sf::Time dt ) = 0;
  virtual std::string get_name() const = 0;

  //! @brief Get the registry object owned by the scene
  //! @return entt::registry&
  virtual entt::registry &get_registry() = 0;

  virtual bool blocks_update() const { return true; }

protected:
  entt::registry m_reg;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_ISCENE_HPP_