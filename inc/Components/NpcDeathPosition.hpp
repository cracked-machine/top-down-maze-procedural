#ifndef __CMP_NPC_DEATH_POSITION_HPP__
#define __CMP_NPC_DEATH_POSITION_HPP__

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace ProceduralMaze::Cmp {

class NpcDeathPosition : public sf::Vector2f
{
public:
  NpcDeathPosition( sf::Vector2f pos ) : sf::Vector2f( pos ) {}
  // keep track of animation frame for death animation;
  // use of this property is MultiSprite-dependent. See RenderGameSystem.cpp
  std::size_t current_anim_frame{ 0 };
  sf::Time elapsed_time{ sf::Time::Zero };
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_NPC_DEATH_POSITION_HPP__