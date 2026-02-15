#ifndef __COMPONENTS_DIRECTION_HPP__
#define __COMPONENTS_DIRECTION_HPP__

#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp
{

// {1,0} right, {-1,0} left, {0,-1} up, {0,1} down
class Direction : public sf::Vector2f
{
public:
  Direction() = default;
  Direction( const sf::Vector2f &dir )
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

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_DIRECTION_HPP__