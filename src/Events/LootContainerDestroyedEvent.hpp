#ifndef INC_EVENTS_LOOTCONTAINERDESTROYEDEVENT_HPP__
#define INC_EVENTS_LOOTCONTAINERDESTROYEDEVENT_HPP__

#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct LootContainerDestroyedEvent
{
  entt::entity m_entity;

  explicit LootContainerDestroyedEvent( entt::entity entity )
      : m_entity( entity )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // INC_EVENTS_LOOTCONTAINERDESTROYEDEVENT_HPP__
