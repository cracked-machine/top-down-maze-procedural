#ifndef INC_COMPONENTS_SPRITEANIMATION_HPP
#define INC_COMPONENTS_SPRITEANIMATION_HPP

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp {

struct SpriteAnimation
{
  // track the frame position in the animation sequence
  // NOTE: for multi block sprites, this is relative to sprite index
  // E.g. for single sprites:
  //    Frame #1: [0], Frame #2: [1], etc.
  //    so 0 is the first block, 1 is the second frame, etc.
  // E.g. for multi block sprites (4 blocks per frame):
  //    Frame #1: [0,1,2,3], Frame #2: [4,5,6,7], etc.
  //    so 0 is the first frame, 4 is the second frame, etc.
  unsigned int m_current_frame = 0;

  // the start frame in the animation sequence
  unsigned int m_base_frame = 0;

  // elapsed time since the last frame change
  sf::Time m_elapsed_time = sf::Time::Zero;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_SPRITEANIMATION_HPP