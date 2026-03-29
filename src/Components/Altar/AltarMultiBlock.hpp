#ifndef SRC_COMPONENTS_ALTARMULTIBLOCK_HPP__
#define SRC_COMPONENTS_ALTARMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// AltarMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its AltarSegment parts and track overall activation state
class AltarMultiBlock : public sf::FloatRect
{
public:
  AltarMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
    m_exitkey_drop_threshold = 4;
    m_cryptkey_drop_threshold = 5;
  }

  void increment_sacrifice_count() { ++m_sacrifice_count; }
  void set_sacrifice_count( uint8_t count ) { m_sacrifice_count = count; }
  uint8_t get_sacrifice_count() const { return m_sacrifice_count; }

  uint8_t get_exitkey_drop_threshold() const { return m_exitkey_drop_threshold; }
  uint8_t get_cryptkey_drop_threshold() const { return m_cryptkey_drop_threshold; }

  // void set_powers_active( bool activate = true ) { m_powers_active = activate; }
  // bool are_powers_active() const { return m_powers_active; }

  void set_exitkey_lockout() { m_exitkey_drop_lockout = true; }
  bool is_exitkey_lockout() const { return m_exitkey_drop_lockout; }

  void set_cryptkey_lockout() { m_cryptkey_drop_lockout = true; }
  bool is_cryptkey_lockout() const { return m_cryptkey_drop_lockout; }

private:
  //! @brief The number of sacrifices required to drop an exitkey (4 relics)
  uint8_t m_exitkey_drop_threshold{ 0 };
  //! @brief The number of sacrifices required to drop an cryptkey (4 relics + 1 exitkey)
  uint8_t m_cryptkey_drop_threshold{ 0 };

  uint8_t m_sacrifice_count{ 0 };

  // // Overall altar multi-block activation. Has multiple meanings depending on context.
  // // The callee is responsible for knowing when to set this. Usually when m_activation_count reaches some
  // // threshold value.
  // bool m_powers_active{ false };

  bool m_exitkey_drop_lockout{ false };
  bool m_cryptkey_drop_lockout{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_ALTARMULTIBLOCK_HPP__