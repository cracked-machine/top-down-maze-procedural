#ifndef SRC_COMPONENTS_WORMHOLE_JUMP_HPP__
#define SRC_COMPONENTS_WORMHOLE_JUMP_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
namespace Game::Cmp::Wormhole
{

class Jump
{
public:
  sf::Clock jump_clock;
  sf::Time jump_cooldown{ sf::seconds( 2.0f ) };
  bool active{ true };
};

} // namespace Game::Cmp::Wormhole

#endif // SRC_COMPONENTS_WORMHOLE_JUMP_HPP__
