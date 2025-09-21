#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

struct NPC
{
  sf::Clock m_damage_cooldown;
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__
