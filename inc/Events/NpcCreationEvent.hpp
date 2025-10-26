#ifndef __EVENTS_NPC_CREATION_EVENT_HPP__
#define __EVENTS_NPC_CREATION_EVENT_HPP__

#include <MultiSprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct NpcCreationEvent
{

  explicit NpcCreationEvent( entt::entity entt )
      : npc_entity( entt )
  {
  }
  entt::entity npc_entity;
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_NPC_CREATION_EVENT_HPP__