#ifndef SRC_COMPONENTS_RUIN_ENTRANCE_HPP__
#define SRC_COMPONENTS_RUIN_ENTRANCE_HPP__

namespace Game::Cmp::Ruin
{

//! @brief Component for a ruin entrance door, used for collision detection so the player can trigger the
//! scene transition into the ruin.
class Entrance
{
public:
  //! @brief Construct a new Entrance.
  //! @param is_open Initial open/closed state of the door.
  Entrance( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  //! @brief Get whether the door is currently open.
  //! @return bool
  bool is_open() const { return m_is_open; }
  //! @brief Set whether the door is open.
  //! @param open
  void set_is_open( bool open ) { m_is_open = open; }

private:
  //! @brief Whether the door is currently open.
  bool m_is_open{ true };
};

} // namespace Game::Cmp::Ruin

#endif // SRC_COMPONENTS_RUIN_ENTRANCE_HPP__
