#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <spdlog/spdlog.h>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp {

// See PlayerSystem.hpp for initilization values
class PlayableCharacter
{
public:
  // 0 is no bombs, -1 is infinite bombs
  int bomb_inventory{};
  // blast radius for bomb explosions
  int blast_radius{};

  // prevent bomb spamming
  sf::Clock m_bombdeploycooldowntimer;
  // prevent bomb spamming
  const sf::Time m_bombdeploydelay{ sf::seconds( 0.5 ) };
  // prevent placing more than one bomb at a time

  bool has_active_bomb{ false };

  bool underwater{ false };

  sf::Clock m_damage_cooldown_timer;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__