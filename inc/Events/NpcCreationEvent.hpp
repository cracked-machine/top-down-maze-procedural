#ifndef __EVENTS_NPC_CREATION_EVENT_HPP__
#define __EVENTS_NPC_CREATION_EVENT_HPP__

#include <SFML/System/Vector2.hpp>
#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct NpcCreationEvent
{
  sf::Vector2f position;
  explicit NpcCreationEvent( sf::Vector2f pos )
      : position( pos )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // __EVENTS_NPC_CREATION_EVENT_HPP__