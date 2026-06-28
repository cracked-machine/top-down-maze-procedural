#ifndef SRC_COMPONENTS_ANIMDATA_HPP__
#define SRC_COMPONENTS_ANIMDATA_HPP__

#include <SFML/System/Time.hpp>
#include <Sprites/SpriteMetaType.hpp>

namespace Game::Cmp
{

enum class AnimType { LOOP, ONESHOTRESET, ONESHOTHOLD };

struct AnimData
{
  struct Config
  {
    Sprites::SpriteMetaType sprite_type;
    unsigned int current_frame = 0;
    unsigned int base_frame = 0;
    size_t frame_index_offset = 0;
    float framerate = 0.1f;
    bool enabled = true;
    AnimType anim_type = AnimType::LOOP;
  };

  explicit AnimData( const Config &cfg )
      : m_current_frame( cfg.current_frame ),
        m_base_frame( cfg.base_frame ),
        m_enabled( cfg.enabled ),
        m_sprite_type( cfg.sprite_type ),
        m_anim_type( cfg.anim_type ),
        m_framerate( cfg.framerate ),
        m_frame_index_offset( cfg.frame_index_offset )
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

  bool m_enabled;

  //! @brief The SpriteSheet object type (string) that we are animating
  Sprites::SpriteMetaType m_sprite_type;

  //! @brief Get the frame index offset for multi-block sprites
  //! @return unsigned int
  unsigned int getFrameIndexOffset() const { return m_frame_index_offset; }

  AnimType m_anim_type;

  float get_framerate() { return m_framerate; }

private:
  float m_framerate;
  //! @brief Get the relative frame index offset within multi-block sprites
  //! This should be immutable for the lifetime of the component
  //! @return unsigned int
  size_t m_frame_index_offset;
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_ANIMDATA_HPP__
