#ifndef INC_EVENTS_ANIMDIRECTIONCHANGEEVENT_HPP__
#define INC_EVENTS_ANIMDIRECTIONCHANGEEVENT_HPP__

#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct AnimDirectionChangeEvent
{
  entt::entity m_entity;

  explicit AnimDirectionChangeEvent( entt::entity entity )
      : m_entity( entity )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // INC_EVENTS_ANIMRESETEVENT_HPP__