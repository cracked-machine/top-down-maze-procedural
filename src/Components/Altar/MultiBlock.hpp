#ifndef SRC_COMPONENTS_ALTAR_MULTIBLOCK_HPP__
#define SRC_COMPONENTS_ALTAR_MULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace Game::Cmp::Altar
{

// MultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its Segment parts and track overall activation state
class MultiBlock : public sf::FloatRect
{
public:
  MultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
    m_exitkey_drop_threshold = 4;
    m_cryptkey_drop_threshold = 5;
  }

  void increment_sacrifice_count() { ++m_sacrifice_count; }
  void set_sacrifice_count( uint8_t count ) { m_sacrifice_count = count; }
  [[nodiscard]] uint8_t get_sacrifice_count() const { return m_sacrifice_count; }

  [[nodiscard]] uint8_t get_exitkey_drop_threshold() const { return m_exitkey_drop_threshold; }
  [[nodiscard]] uint8_t get_cryptkey_drop_threshold() const { return m_cryptkey_drop_threshold; }

  void set_exitkey_lockout() { m_exitkey_drop_lockout = true; }
  bool is_exitkey_lockout() const { return m_exitkey_drop_lockout; }

  void set_cryptkey_lockout() { m_cryptkey_drop_lockout = true; }
  bool is_cryptkey_lockout() const { return m_cryptkey_drop_lockout; }

  std::array<sf::Vector2f, 4> flame_offsets = { sf::Vector2f( 6.f, 10.f ), sf::Vector2f( 26.f, 10.f ), sf::Vector2f( 6.f, 24.f ),
                                                sf::Vector2f( 26.f, 24.f ) };

private:
  //! @brief The number of sacrifices required to drop an exitkey (4 relics)
  uint8_t m_exitkey_drop_threshold{ 0 };
  //! @brief The number of sacrifices required to drop an cryptkey (4 relics + 1 exitkey)
  uint8_t m_cryptkey_drop_threshold{ 0 };

  uint8_t m_sacrifice_count{ 0 };

  bool m_exitkey_drop_lockout{ false };
  bool m_cryptkey_drop_lockout{ false };
};

} // namespace Game::Cmp::Altar

#endif // SRC_COMPONENTS_ALTAR_MULTIBLOCK_HPP__
