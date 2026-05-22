#ifndef MUSIC_ITEM_HPP
#define MUSIC_ITEM_HPP

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

#endif // MUSIC_ITEM_HPP