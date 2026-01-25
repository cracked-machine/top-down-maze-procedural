#ifndef SRC_COMPONENTS_CRYPTOBJECTIVEMULTIBLOCK_HPP__
#define SRC_COMPONENTS_CRYPTOBJECTIVEMULTIBLOCK_HPP__

#include <SFML/Graphics/RectangleShape.hpp>

#include <Components/Obstacle.hpp>

namespace ProceduralMaze::Cmp
{

// CryptObjectiveMultiBlock component represents a multi-block sprite defined geometrically as a rectangle
// It can be used to find its CryptSegment parts and track overall activation state
class CryptObjectiveMultiBlock : public sf::FloatRect
{
public:
  CryptObjectiveMultiBlock( const sf::Vector2f &position, const sf::Vector2f &size )
      : sf::FloatRect( position, size )
  {
    m_activation_threshold = 1;
  }

  void increment_activation_count() { ++m_activation_count; }
  void set_activation_count( uint8_t count ) { m_activation_count = count; }
  uint8_t get_activation_count() const { return m_activation_count; }

  uint8_t get_activation_threshold() const { return m_activation_threshold; }

private:
  // Track the number of activated sprites in this crypt multi-block
  // Callees are responsible for managing max thresholds
  uint8_t m_activation_threshold{ 0 };
  uint8_t m_activation_count{ 0 };
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_COMPONENTS_CRYPTOBJECTIVEMULTIBLOCK_HPP__