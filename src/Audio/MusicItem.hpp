#ifndef MUSIC_ITEM_HPP
#define MUSIC_ITEM_HPP

#include <SFML/Audio/Music.hpp>

namespace ProceduralMaze::Audio {

struct MusicData
{
  //! @brief Music control
  sf::Music control;

  MusicData( const char *filepath );
};

} // namespace ProceduralMaze::Audio

#endif // MUSIC_ITEM_HPP