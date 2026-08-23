#ifndef SRC_COMPONENTS_VOIDPOSITION_HPP__
#define SRC_COMPONENTS_VOIDPOSITION_HPP__

#include <entt/entity/fwd.hpp>

#include <SFML/System/Vector2.hpp>

#include <Components/Position.hpp>
#include <Factory/SpriteFactory.hpp>

namespace Game::Cmp
{

//! @brief Empty Space for level gen
class VoidPosition : public Cmp::Position
{
public:
  //! @brief Construct a void position from an existing Position component.
  //! @param pos The position and size to mark as empty space.
  VoidPosition( Cmp::Position pos )
      : Position( pos.position, pos.size )
  {
  }

  //! @brief Construct a void position from raw position and size vectors.
  //! @param pos The grid position, in pixels.
  //! @param size The grid square size, in pixels.
  VoidPosition( sf::Vector2f pos, sf::Vector2f size )
      : Position( pos, size )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_VOIDPOSITION_HPP__
