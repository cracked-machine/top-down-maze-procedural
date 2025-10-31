#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <SFML/System/Clock.hpp>

#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp {

struct NPC
{
  NPC( Sprites::SpriteMetaType type, std::size_t tile_index = 0 )
      : m_type( type ),
        m_tile_index( tile_index )
  {
  }
  Sprites::SpriteMetaType m_type;
  unsigned int m_tile_index{ 0 };
  sf::Clock m_damage_cooldown;
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__
