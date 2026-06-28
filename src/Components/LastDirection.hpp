#ifndef SRC_COMPONENTS_LASTDIRECTION_HPP__
#define SRC_COMPONENTS_LASTDIRECTION_HPP__

#include <SFML/System/Vector2.hpp>

namespace Game::Cmp
{

//! @brief This is basically the same as Cmp::Direction with the exeception that unlike Cmp::Direction,
//!        Cmp::LastDirection does not get reset when the player stops moving. Therefore, you can use
//!        Cmp::LastDirection to always tell which way the player is facing, even when standing still.
//! @note  {1,0} right, {-1,0} left, {0,-1} up, {0,1} down
class LastDirection : public sf::Vector2f
{
public:
  LastDirection() = default;
  LastDirection( const sf::Vector2f &dir )
      : sf::Vector2f( dir )
  {
  }

  // used for sprite flipping:
  // 1.f for moving right, -1.f for moving left
  // default: 1.f otherwise we get no sprite until we move
  float x_scale = 1.f;
  // used for sprite positioning after flip:
  // 0 for moving right, BaseSystem::kGridSquareSizePixels.x for moving left
  float x_offset = 0.f;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_LASTDIRECTION_HPP__
