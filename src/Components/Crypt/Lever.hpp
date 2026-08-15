#ifndef SRC_COMPONENTS_CRYPT_LEVER_HPP__
#define SRC_COMPONENTS_CRYPT_LEVER_HPP__

namespace Game::Cmp::Crypt
{

// Mainly used to distinguish between 16x16 block crypt segments for the purpose of collision detection
class Lever
{
public:
  Lever()
      : m_enabled( false )
  {
  }

  bool isEnabled() const { return m_enabled; }
  void setEnabled( bool enabled ) { m_enabled = enabled; }

private:
  // Is collision detection enabled for this sprite
  bool m_enabled{ false };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_LEVER_HPP__
