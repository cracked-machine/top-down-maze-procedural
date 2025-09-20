#ifndef __SYS_MUSIC_SYSTEM_HPP__
#define __SYS_MUSIC_SYSTEM_HPP__

#include <BaseSystem.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace ProceduralMaze::Sys {

class MusicSystem : public BaseSystem
{

public:
  MusicSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg, std::filesystem::path music_path );

  void init_context();

  enum class Function
  {
    PLAY,
    STOP
  };

  void update_music_playback( Function func );
  void start_music_fade_out();
  void update_volume();
  bool is_fading_out() const { return m_music_fading; }

private:
  // we're using sf::Sound instead of sf::Music because it guarantees smooth playback without
  // stuttering
  sf::SoundBuffer m_music_buffer{};
  sf::Sound m_music{ m_music_buffer };

  // This is the volume that the fade starts at, it is captured from the current volume when the
  // fade starts
  float m_prefade_volume{};
  // State for volume fade event
  bool m_music_fading = false;
  // used to time the fade
  sf::Clock m_music_fade_clock;
  // Duration in seconds for audio fade transitions.
  // This constant defines the time it takes for music to fade in or fade out
  // when transitioning between tracks or adjusting volume levels.
  static constexpr float kFadeDuration = 0.25f;
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_MUSIC_SYSTEM_HPP__