#ifndef SRC_SCENECONTROL_SCENE_HPP__
#define SRC_SCENECONTROL_SCENE_HPP__

#include <PathFinding/SmartPointers.hpp>
#include <SceneControl/IScene.hpp>
#include <SceneControl/SmartPointers.hpp>

// clang-format off
namespace PathFinding { class SpatialHashGrid; }
// clang-format on

namespace Game::Scene
{

//! @brief Base class for all scenes that process input events of a specific type
//! @tparam InputEventType The type of input event to process. This essentially means we can "bake in"
//!         the call to `m_nav_event_dispatcher.enqueue<InputEventType>()` after each update, so that the
//!         specialization of this class template doesn't have to remember to do that.
template <typename InputEventType>
class Scene : public IScene
{
public:
  //! @brief Construct a new Scene object
  //! @param nav_event_dispatcher Used to notify the SceneInputRouter when new input events are available
  explicit Scene( entt::dispatcher &nav_event_dispatcher )
      : m_nav_event_dispatcher( nav_event_dispatcher )
  {
  }

  //! @brief Destroy the Scene object
  virtual ~Scene() = default;

  //! @brief Public API for updating the scene
  //! @param dt The time elapsed since the last update
  void update( sf::Time dt ) override
  {
    // call the user-overridden update function
    do_update( dt );

    // Notify SceneInputRouter that there may be new input events to process
    m_nav_event_dispatcher.enqueue( InputEventType() );
  }

protected:
  PathFinding::SpatialHashGridSharedPtr m_npc_navmesh;
  PathFinding::SpatialHashGridSharedPtr m_player_navmesh;
  PathFinding::SpatialHashGridSharedPtr m_open_navmesh;
  SceneMapSharedPtr m_scene_data;
  bool m_just_spawned{ true };

private:
  entt::dispatcher &m_nav_event_dispatcher;
};

} // namespace Game::Scene

#endif // SRC_SCENECONTROL_SCENE_HPP__
