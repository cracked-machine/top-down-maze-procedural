#ifndef SCENE_ISCENE_HPP_
#define SCENE_ISCENE_HPP_

#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>

namespace ProceduralMaze::Scene
{

enum class SceneRequest
{
  None,
  StartGame,
  OpenSettings,
  Pause,
  Resume,
  GameOver,
  LevelComplete,
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

  void set_scene_manager( SceneManager *sm ) { m_scene_manager = sm; }
  virtual entt::registry *get_registry() = 0;

  void request( SceneRequest r ) { requestFlag = r; }
  SceneRequest consume_request()
  {
    SceneRequest r = requestFlag;
    requestFlag = SceneRequest::None;
    return r;
  }

  virtual bool blocks_update() const { return true; }

protected:
  SceneManager *m_scene_manager = nullptr; // non-owning
  entt::registry registry;

private:
  SceneRequest requestFlag = SceneRequest::None;
};

} // namespace ProceduralMaze::Scene

#endif // SCENE_ISCENE_HPP_