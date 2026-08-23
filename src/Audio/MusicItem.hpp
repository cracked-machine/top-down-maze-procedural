#ifndef SRC_AUDIO_MUSICITEM_HPP__
#define SRC_AUDIO_MUSICITEM_HPP__

#include <SFML/Audio/Music.hpp>

namespace Game::Audio
{

//! @brief Owns the playback control for a single registered music track.
struct MusicData
{
  //! @brief Music control
  sf::Music control;

  //! @brief Construct a new Music Data object, opening the track from `filepath` for streaming.
  //! @param filepath Path to the music audio file.
  MusicData( const std::filesystem::path &filepath );
};

} // namespace Game::Audio

#endif // SRC_AUDIO_MUSICITEM_HPP__
