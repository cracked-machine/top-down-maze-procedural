#ifndef INC_EVENTS_ANIMRESETEVENTFRAME_HPP__
#define INC_EVENTS_ANIMRESETEVENTFRAME_HPP__

#include <Direction.hpp>
#include <SpriteAnimation.hpp>
#include <entt/entity/entity.hpp>

namespace ProceduralMaze::Events {

struct AnimResetFrameEvent
{
  entt::entity m_entity;

  explicit AnimResetFrameEvent( entt::entity entity )
      : m_entity( entity )
  {
  }
};

} // namespace ProceduralMaze::Events

#endif // INC_EVENTS_ANIMRESETEVENTFRAME_HPP__