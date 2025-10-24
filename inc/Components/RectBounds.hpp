#ifndef __CMP_RECTBOUNDS_HPP__
#define __CMP_RECTBOUNDS_HPP__

#include <BaseSystem.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <Sprites/SpriteFactory.hpp>
#ifdef _WIN32
#include <windows.h>
#endif

namespace ProceduralMaze::Cmp {

/**
 * @brief A wrapper class for sf::FloatRect that provides scaled bounds with position offset calculations.
 *
 * RectBounds manages a rectangular boundary with automatic scaling and position offsetting
 * based on sprite dimensions. It provides convenient methods for updating position and size
 * while maintaining the scaling factor and offset calculations.
 *
 * The position offset is calculated to center the bounds relative to sprite dimensions,
 * taking into account the scale factor.
 */
class RectBounds
{
public:
  enum class ScaleCardinality
  {
    HORIZONTAL,
    VERTICAL,
    BOTH
  };

  /**
   * @brief Constructs a RectBounds object with specified position, size, and scale factor.
   *
   * Creates a rectangular bounds object by scaling the provided size and adjusting the position
   * based on default sprite dimensions and a position offset.
   *
   * @param position The initial position vector for the bounds
   * @param size The base size vector before scaling is applied
   * @param ScaleFactor The scaling factor to apply to the size dimensions
   */
  RectBounds( sf::Vector2f position, sf::Vector2f size, float ScaleFactor,
              ScaleCardinality scale_cardinality = ScaleCardinality::BOTH )
      : m_scale_factor( ScaleFactor ),
        m_scale_cardinality( scale_cardinality )
  {
    // calculate the bounds scale/offset for requested cardinality
    switch ( m_scale_cardinality )
    {
      case ScaleCardinality::HORIZONTAL:
        m_bounds.size.x = size.x * m_scale_factor;
        m_bounds.size.y = size.y * 1;
        m_bounds.position.x = position.x - Sys::BaseSystem::kGridSquareSizePixels.x * kPositionOffsetFactor;
        m_bounds.position.y = position.y;
        break;
      case ScaleCardinality::VERTICAL:
        m_bounds.size.y = size.y * m_scale_factor;
        m_bounds.size.x = size.x * 1;
        m_bounds.position.y = position.y - Sys::BaseSystem::kGridSquareSizePixels.y * kPositionOffsetFactor;
        m_bounds.position.x = position.x;
        break;
      case ScaleCardinality::BOTH:
      default:
        m_bounds.size = size * m_scale_factor;
        m_bounds.position = position - sf::Vector2f{ Sys::BaseSystem::kGridSquareSizePixels } * kPositionOffsetFactor;
        break;
    }
  }

  /**
   * @brief Sets the position of the rectangular bounds
   *
   * Updates the bounds position by applying an offset calculation based on the new position.
   * The offset is calculated using the default sprite dimensions and `kPositionOffsetFactor`.
   *
   * @param new_position The new position to set for the bounds
   */
  void position( sf::Vector2f new_position )
  {
    m_bounds.position = new_position - sf::Vector2f{ Sys::BaseSystem::kGridSquareSizePixels } * kPositionOffsetFactor;
  }
  sf::Vector2f position() const { return m_bounds.position; }

  /**
   * @brief Sets the size of the rectangular bounds.
   *
   * Updates the internal bounds size by applying the scale factor to the provided dimensions.
   * The actual size stored will be the new_size multiplied by the current scale factor.
   *
   * @param new_size The desired size as a 2D vector (width, height) before scaling
   */
  void size( sf::Vector2f new_size ) { m_bounds.size = new_size * m_scale_factor; }
  sf::Vector2f size() const { return m_bounds.size; }

  std::optional<sf::FloatRect> findIntersection( const sf::FloatRect &rectangle )
  {
    return m_bounds.findIntersection( rectangle );
  }

  sf::FloatRect getBounds() const { return m_bounds; }

private:
  /**
   * @brief The rectangular bounds of the component in world coordinates.
   *
   * This FloatRect defines the spatial boundaries of the component, typically used
   * for collision detection, rendering bounds, or spatial queries. The rectangle
   * is represented with floating-point precision for accurate positioning.
   *
   * @note The bounds are usually in world space coordinates and may be updated
   *       when the component's position or size changes.
   */
  sf::FloatRect m_bounds;
  /**
   * @brief Scale factor used to modify the size of the rectangular bounds.
   *
   * This factor is multiplied with the base dimensions to determine the actual
   * size of the rectangular bounds. A value of 1.0 represents the original size,
   * values greater than 1.0 increase the size, and values less than 1.0 decrease it.
   */
  float m_scale_factor;
  /**
   * @brief Offset value used to center objects based on scale factor.
   *
   * Calculates the position offset needed to properly center an object when its scale
   * factor differs from 1.0. The offset is computed as half the scale factor minus 0.5,
   * which accounts for the difference between the object's scaled size and its default size.
   */
  const float kPositionOffsetFactor{ ( m_scale_factor / 2.f ) - 0.5f };

  ScaleCardinality m_scale_cardinality;
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_RECTBOUNDS_HPP__
