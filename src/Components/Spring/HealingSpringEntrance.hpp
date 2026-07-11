#ifndef SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__
#define SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__

namespace Game::Cmp
{

// Used for collision detection on crypt doors
class HealingSpringEntrance
{
public:
  HealingSpringEntrance( bool is_open = false )
      : m_is_open( is_open )
  {
  }

  [[nodiscard]] bool is_open() const { return m_is_open; }
  void set_is_open( bool open ) { m_is_open = open; }

private:
  // Is collision detection enabled for this sprite
  bool m_is_open{ true };
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_SPRING_SACREDSPRINGENTRANCE_HPP__
