#ifndef INC_COMPONENTS_SPRITEANIMATION_HPP
#define INC_COMPONENTS_SPRITEANIMATION_HPP

#include <SFML/System/Time.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp {

struct SpriteAnimation
{
  SpriteAnimation( unsigned int current_frame = 0, unsigned int base_frame = 0, bool activate_animation = true,
                   Sprites::SpriteMetaType sprite_type = "" )
      : m_current_frame( current_frame ),
        m_base_frame( base_frame ),
        m_animation_active( activate_animation ),
        m_sprite_type( sprite_type )
  {
  }

  // track the frame position in the animation sequence
  // NOTE: for multi block sprites, this is relative to sprite index
  // E.g. for single sprites:
  //    Frame #1: [0], Frame #2: [1], etc.
  //    so 0 is the first block, 1 is the second frame, etc.
  // E.g. for multi block sprites (4 blocks per frame):
  //    Frame #1: [0,1,2,3], Frame #2: [4,5,6,7], etc.
  //    so 0 is the first frame, 4 is the second frame, etc.
  unsigned int m_current_frame;

  // the start frame in the animation sequence
  unsigned int m_base_frame;

  // elapsed time since the last frame change
  sf::Time m_elapsed_time{ sf::Time::Zero };

  bool m_animation_active;

  //! @brief State for the current animation sequence
  Sprites::SpriteMetaType m_sprite_type = "";
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_SPRITEANIMATION_HPP