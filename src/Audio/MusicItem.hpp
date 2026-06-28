#ifndef SRC_AUDIO_MUSICITEM_HPP__
#define SRC_AUDIO_MUSICITEM_HPP__

#include <SFML/Audio/Music.hpp>

namespace Game::Audio
{

struct MusicData
{
  //! @brief Music control
  sf::Music control;

  MusicData( const std::filesystem::path &filepath );
};

} // namespace Game::Audio

#endif // SRC_AUDIO_MUSICITEM_HPP__
