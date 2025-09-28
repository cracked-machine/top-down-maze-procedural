#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <Base.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// See PlayerSystem.hpp for initilization values
class PlayableCharacter
{
public:
  // 0 is no bombs, -1 is infinite bombs
  int bomb_inventory{};
  // blast radius for bomb explosions
  int blast_radius{};

  // the source of all truthiness
  bool alive{ true };
  // player health, signed, to prevent wrap-around
  int8_t health{ 100 };

  // prevent bomb spamming
  sf::Clock m_bombdeploycooldowntimer;
  // prevent bomb spamming
  const sf::Time m_bombdeploydelay{ sf::seconds( 0.5 ) };
  // prevent placing more than one bomb at a time
  bool has_active_bomb{ false };

  bool underwater{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__