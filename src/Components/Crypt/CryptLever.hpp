#ifndef SRC_COMPONENTS_CRYPT_CRYPTLEVER_HPP__
#define SRC_COMPONENTS_CRYPT_CRYPTLEVER_HPP__

namespace Game::Cmp
{

// Mainly used to distinguish between 16x16 block crypt segments for the purpose of collision detection
class CryptLever
{
public:
  CryptLever()
      : m_enabled( false )
  {
  }

  bool isEnabled() const { return m_enabled; }
  void setEnabled( bool enabled ) { m_enabled = enabled; }

private:
  // Is collision detection enabled for this sprite
  bool m_enabled{ false };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_CRYPT_CRYPTLEVER_HPP__
