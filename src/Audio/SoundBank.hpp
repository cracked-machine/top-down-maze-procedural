#ifndef __SYS__FXSYSTEM_HPP__
#define __SYS__FXSYSTEM_HPP__

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <spdlog/spdlog.h>

#include <Audio/EffectsItem.hpp>
#include <Audio/MusicItem.hpp>

#include <unordered_map>

namespace ProceduralMaze::Audio {

class SoundBank
{
public:
  SoundBank();
  void init();
  void update_effects_volume( float volume );
  void update_music_volume( float volume );
  sf::Sound &get_effect( const std::string &name );
  sf::Music &get_music( const std::string &name );

private:
  static sf::SoundBuffer generate_tone( float frequency = 440.0f, float duration = 0.5f,
                                        unsigned int sample_rate = 44100 );

  std::unordered_map<std::string, EffectsData> sounds;
  std::unordered_map<std::string, MusicData> music;
};

} // namespace ProceduralMaze::Audio

#endif // __SYS__FXSYSTEM_HPP__