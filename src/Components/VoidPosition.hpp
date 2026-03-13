#ifndef SRC_CMPS_VOIDPOSITION_HPP_
#define SRC_CMPS_VOIDPOSITION_HPP_

#include <entt/entity/fwd.hpp>

#include <SFML/System/Vector2.hpp>

#include <Components/Position.hpp>
#include <Sprites/SpriteFactory.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Empty Space for level gen
class VoidPosition : public Cmp::Position
{
public:
  VoidPosition( Cmp::Position pos )
      : Position( pos.position, pos.size )
  {
  }

  VoidPosition( sf::Vector2f pos, sf::Vector2f size )
      : Position( pos, size )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_VOIDPOSITION_HPP_