#ifndef __EVENTS_NPC_DEATH_EVENT_HPP__
#define __EVENTS_NPC_DEATH_EVENT_HPP__

#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct NpcDeathEvent
{
  entt::entity npc_entity;
  explicit NpcDeathEvent( entt::entity entity ) : npc_entity( entity ) {}
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_NPC_DEATH_EVENT_HPP__