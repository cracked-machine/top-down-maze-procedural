#ifndef SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__
#define SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__

#include <Components/Position.hpp>
namespace Game::Cmp::Ruin
{

class FloorAccess : public Cmp::Position
{

public:
  enum class Direction { TO_LOWER, TO_UPPER };
  FloorAccess( sf::Vector2f pos, sf::Vector2f size, Direction direction )
      : Cmp::Position( pos, size ),
        m_direction( direction )
  {
  }
  Direction m_direction;
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_FLOORACCESS_HPP__
