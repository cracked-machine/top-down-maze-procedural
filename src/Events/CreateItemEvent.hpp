#ifndef SRC_EVENTS_CREATEITEMEVENT_HPP_
#define SRC_EVENTS_CREATEITEMEVENT_HPP_

#include <Components/Position.hpp>
#include <utility>

namespace Game::Events
{

struct CreateItemEvent
{

  explicit CreateItemEvent( Cmp::Position pos, std::string item, std::string sfx = "", float zorder = 0.f )
      : m_pos( pos ),
        m_item( std::move( item ) ),
        m_sfx( std::move( sfx ) ),
        m_zorder( zorder )
  {
  }

  Cmp::Position m_pos;
  std::string m_item;
  std::string m_sfx;
  float m_zorder;
};

} // namespace Game::Events

#endif // SRC_EVENTS_CREATEITEMEVENT_HPP_