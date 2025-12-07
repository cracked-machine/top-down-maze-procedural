#ifndef SCENE_ISCENE_HPP_
#define SCENE_ISCENE_HPP_

#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <string>

namespace ProceduralMaze::Scene
{

enum class SceneRequest
{
  None,
  SettingsMenu,
  GraveyardScene,
  PausedMenu,
  GameOver,
  LevelComplete,
  PopOverlay,
  Pop,
  Quit
};

//! @brief Forward declaration of SceneManager to avoid circular dependency
class SceneManager;

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