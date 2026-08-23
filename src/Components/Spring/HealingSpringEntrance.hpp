#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__

namespace Game::Cmp
{

//! @brief Marks the entrance/door of a healing spring building, used for collision detection on the door.
class HealingSpringEntrance
{
public:
  //! @brief Construct a new HealingSpringEntrance object.
  //! @param is_open Whether the entrance starts open.
  HealingSpringEntrance( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  //! @brief Check whether the entrance is currently open.
  //! @return bool true if open.
  [[nodiscard]] bool is_open() const { return m_is_open; }

  //! @brief Set whether the entrance is open.
  //! @param open true to open the entrance.
  void set_is_open( bool open ) { m_is_open = open; }

private:
  //! @brief Is collision detection enabled for this sprite (i.e. is the entrance closed/blocking).
  bool m_is_open{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__
