#ifndef SRC_COMPONENTS_RUIN_RUINFLOORACCESS_HPP__
#define SRC_COMPONENTS_RUIN_RUINFLOORACCESS_HPP__

#include <Components/Position.hpp>
namespace Game::Cmp
{

class RuinFloorAccess : public Cmp::Position
{

public:
  enum class Direction { TO_LOWER, TO_UPPER };
  RuinFloorAccess( sf::Vector2f pos, sf::Vector2f size, Direction direction )
      : Cmp::Position( pos, size ),
        m_direction( direction )
  {
  }
  Direction m_direction;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_RUIN_RUINFLOORACCESS_HPP__
