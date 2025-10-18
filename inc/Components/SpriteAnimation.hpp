#ifndef INC_COMPONENTS_SPRITEANIMATION_HPP
#define INC_COMPONENTS_SPRITEANIMATION_HPP

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp {

struct SpriteAnimation
{
  // track the frame position in the animation sequence
  unsigned int m_current_frame = 0;

  // the start frame in the animation sequence
  unsigned int m_base_frame = 0;

  // elapsed time since the last frame change
  sf::Time m_elapsed_time = sf::Time::Zero;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_SPRITEANIMATION_HPP