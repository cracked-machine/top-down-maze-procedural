#ifndef SRC_COMPONENTS_GRAVEMULTIBLOCK_HPP__
#define SRC_COMPONENTS_GRAVEMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// GraveMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its GraveSegment parts and track overall activation state
class GraveMultiBlock : public sf::FloatRect
{
public:
  GraveMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
    m_activation_threshold = ( size.x / 16 ) * ( size.y / 16 ); // each block is 16x16 pixels
  }

  void increment_activation_count() { ++m_activation_count; }
  void set_activation_count( uint8_t count ) { m_activation_count = count; }
  uint8_t get_activation_count() const { return m_activation_count; }

  uint8_t get_activation_threshold() const { return m_activation_threshold; }

  void set_powers_active( bool activate = true ) { m_powers_active = activate; }
  bool are_powers_active() const { return m_powers_active; }

private:
  // Track the number of activated sprites in this altar multi-block
  // Callees are responsible for managing max thresholds
  uint8_t m_activation_threshold{ 0 };
  uint8_t m_activation_count{ 0 };

  // Overall altar multi-block activation. Has multiple meanings depending on context.
  // The callee is responsible for knowing when to set this. Usually when m_activation_count reaches some
  // threshold value.
  bool m_powers_active{ false };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_GRAVEMULTIBLOCK_HPP__