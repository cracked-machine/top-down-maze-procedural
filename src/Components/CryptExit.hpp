#ifndef SRC_COMPONENTS_CRYPTEXIT_HPP__
#define SRC_COMPONENTS_CRYPTEXIT_HPP__

#include <SFML/System/Clock.hpp>
#include <Sprites/MultiSprite.hpp>

namespace ProceduralMaze::Cmp
{

// Used for collision detection on crypt doors
class CryptExit
{
public:
  CryptExit( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  bool is_open() const { return m_is_open; }
  void set_is_open( bool open ) { m_is_open = open; }

private:
  // Is collision detection enabled for this sprite
  bool m_is_open{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTEXIT_HPP__