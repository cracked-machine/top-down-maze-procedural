#ifndef SRC_COMPONENTS_DIRECTION_HPP__
#define SRC_COMPONENTS_DIRECTION_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief Component storing an entity's current movement direction as a unit-ish vector:
//!        {1,0} right, {-1,0} left, {0,-1} up, {0,1} down. Reset to zero when the entity stops moving.
class Direction : public sf::Vector2f
{
public:
  Direction() = default;
  //! @brief Construct from a direction vector.
  //! @param dir Movement direction: {1,0} right, {-1,0} left, {0,-1} up, {0,1} down.
  Direction( const sf::Vector2f &dir )
      : sf::Vector2f( dir )
  {
  }

  //! @brief Used for sprite flipping: 1.f for moving right, -1.f for moving left.
  //! @note Defaults to 1.f, otherwise we get no sprite until we move.
  float x_scale = 1.f;
  //! @brief Used for sprite positioning after flip: 0 for moving right,
  //!        BaseSystem::kGridSquareSizePixels.x for moving left.
  float x_offset = 0.f;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_DIRECTION_HPP__
