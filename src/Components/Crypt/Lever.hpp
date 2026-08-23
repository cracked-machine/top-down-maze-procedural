#ifndef SRC_COMPONENTS_CRYPT_LEVER_HPP__
#define SRC_COMPONENTS_CRYPT_LEVER_HPP__

namespace Game::Cmp::Crypt
{

//! @brief Marks an entity as a crypt lever that the player can activate, used by Systems::CryptSystem to
//! track how many levers have been triggered while shuffling/unlocking Cmp::Crypt::RoomOpen areas.
class Lever
{
public:
  //! @brief Construct a new Lever, initially disabled.
  Lever()
      : m_enabled( false )
  {
  }

  //! @brief Get whether the lever has been activated.
  //! @return bool
  bool isEnabled() const { return m_enabled; }
  //! @brief Set whether the lever has been activated.
  //! @param enabled
  void setEnabled( bool enabled ) { m_enabled = enabled; }

private:
  //! @brief Whether this lever has been activated by the player.
  bool m_enabled{ false };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_LEVER_HPP__
