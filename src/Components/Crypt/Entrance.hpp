#ifndef SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__
#define SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__

namespace Game::Cmp::Crypt
{

//! @brief Open/closed state for a crypt door entity, used for collision-based detection on crypt doors
//! (e.g. the graveyard entrance into the crypt, and the crypt's own locked door).
class Entrance
{
public:
  //! @brief Construct a new Entrance.
  //! @param is_open Initial open state.
  Entrance( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  //! @brief Get whether the door is open.
  //! @return bool
  bool is_open() const { return m_is_open; }
  //! @brief Set whether the door is open.
  //! @param open
  void set_is_open( bool open ) { m_is_open = open; }

private:
  //! @brief Whether the door is currently open.
  bool m_is_open{ false };
};

} // namespace Game::Cmp::Crypt

#endif // SRC_COMPONENTS_CRYPT_ENTRANCE_HPP__
