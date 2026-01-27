#ifndef SRC_CMPS_RUIN_RUINFLOORACCESS_HPP_
#define SRC_CMPS_RUIN_RUINFLOORACCESS_HPP_

#include <Components/Position.hpp>
namespace ProceduralMaze::Cmp
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

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_RUIN_RUINFLOORACCESS_HPP_