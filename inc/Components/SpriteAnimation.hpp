#ifndef INC_COMPONENTS_SPRITEANIMATION_HPP
#define INC_COMPONENTS_SPRITEANIMATION_HPP

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp {

struct SpriteAnimation
{
  // Number of frames in animation sequence
  int m_frame_count = 3;

  int m_current_frame = 0;

  // Base frame for current direction
  int m_base_frame = 0;
  sf::Time m_elapsed_time = sf::Time::Zero;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_SPRITEANIMATION_HPP