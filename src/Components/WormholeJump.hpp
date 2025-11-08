#ifndef __CMP_WORMHOLEJUMP_HPP__
#define __CMP_WORMHOLEJUMP_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
namespace ProceduralMaze::Cmp {

class WormholeJump
{
public:
  sf::Clock jump_clock;
  sf::Time jump_cooldown{ sf::seconds( 2.0f ) };
  bool active{ true };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_WORMHOLEJUMP_HPP__