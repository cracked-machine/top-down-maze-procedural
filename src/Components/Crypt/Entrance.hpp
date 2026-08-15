#ifndef SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__
#define SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__

namespace Game::Cmp::Crypt
{

// Used for collision detection on crypt doors
class Entrance
{
public:
  Entrance( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  bool is_open() const { return m_is_open; }
  void set_is_open( bool open ) { m_is_open = open; }

private:
  // Is collision detection enabled for this sprite
  bool m_is_open{ false };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__
