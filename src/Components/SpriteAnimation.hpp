#ifndef INC_COMPONENTS_SPRITEANIMATION_HPP
#define INC_COMPONENTS_SPRITEANIMATION_HPP

#include <SFML/System/Time.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace ProceduralMaze::Cmp
{

struct SpriteAnimation
{
  SpriteAnimation( unsigned int current_frame = 0, unsigned int base_frame = 0, bool activate_animation = true,
                   Sprites::SpriteMetaType sprite_type = "", unsigned int frame_index_offset = 0 )
      : m_current_frame( current_frame ),
        m_base_frame( base_frame ),
        m_animation_active( activate_animation ),
        m_sprite_type( sprite_type ),
        m_frame_index_offset( frame_index_offset )
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

  //! @brief The multisprite object type (string) that we are animating
  Sprites::SpriteMetaType m_sprite_type = "";

  //! @brief Get the frame index offset for multi-block sprites
  //! @return unsigned int
  unsigned int getFrameIndexOffset() const { return m_frame_index_offset; }

private:
  //! @brief Get the relative frame index offset within multi-block sprites
  //! This should be immutable for the lifetime of the component
  //! @return unsigned int
  unsigned int m_frame_index_offset;
};

} // namespace ProceduralMaze::Cmp

#endif // INC_COMPONENTS_SPRITEANIMATION_HPP