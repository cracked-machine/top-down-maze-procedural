#ifndef SRC_UTILS_UTILS_HPP__
#define SRC_UTILS_UTILS_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/View.hpp>
// #include <Systems/BaseSystem.hpp>

#include <cmath>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Constants
{
inline constexpr sf::Vector2u kGridSquareSizePixels{ 16u, 16u };
inline static constexpr sf::Vector2f kGridSquareSizePixelsF{ 16.f, 16.f };
inline static constexpr sf::Vector2u kFallbackDisplaySize{ 1920, 1080 };
} // namespace ProceduralMaze::Constants

namespace ProceduralMaze::Utils
{

//! @brief Snaps a rectangle's position to the nearest grid cell.
//!
//! Computes a new rectangle whose top-left corner is moved to the nearest grid
//! intersection based on BaseSystem::kGridSquareSizePixels.x (the function
//! assumes a square grid and uses the x component as the grid cell size).
//! The input rectangle's size is preserved.
//!
//! Rounding uses std::round, so coordinates are mapped to the nearest multiple
//! of the grid size (negative coordinates are handled correctly by std::round).
//!
//! @param position The rectangle to be snapped. Only its position is considered;
//!                 the rectangle's size remains unchanged.
//! @return A new sf::FloatRect with the snapped position and the original size.
//!
//! @note If the grid is not square, only the x component of
//!       BaseSystem::kGridSquareSizePixels is used for both axes.
inline constexpr sf::FloatRect snap_to_grid( const sf::FloatRect &position ) noexcept
{
  float grid_size = Constants::kGridSquareSizePixels.x; // Assuming square grid
  sf::Vector2f snapped_pos{ std::round( position.position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                            std::round( position.position.y / Constants::kGridSquareSizePixels.y ) * grid_size };

  return sf::FloatRect( snapped_pos, position.size );
}

//! @brief Snap a given position to the nearest grid square.
//! This function takes a 2D position and rounds its coordinates to the nearest
//! grid square based on the grid size defined in BaseSystem::kGridSquareSizePixels.
//! It assumes the grid squares are of uniform size.
//!
//! @param position The position to snap, as an sf::Vector2f.
//! @return sf::Vector2f The snapped position aligned to the grid.
inline constexpr sf::Vector2f snap_to_grid( const sf::Vector2f &position ) noexcept
{
  float grid_size = Constants::kGridSquareSizePixels.x; // Assuming square grid
  sf::Vector2f snapped_pos{ std::round( position.x / Constants::kGridSquareSizePixels.x ) * grid_size,
                            std::round( position.y / Constants::kGridSquareSizePixels.y ) * grid_size };

  return snapped_pos;
}

/**
 * @brief Calculates the bounding rectangle of the given SFML view.
 *
 * This function computes the world-space bounds represented by the specified sf::View.
 * The returned sf::FloatRect describes the area visible through the view.
 *
 * @param view Reference to the SFML view whose bounds are to be calculated.
 * @return sf::FloatRect The rectangle representing the view's bounds in world coordinates.
 */
constexpr inline sf::FloatRect calculate_view_bounds( const sf::View &view )
{
  return sf::FloatRect( view.getCenter() - view.getSize() / 2.f, view.getSize() );
}

/**
 * @brief Determines if a given position rectangle is visible within the specified view bounds.
 *
 * This function checks whether the provided position rectangle intersects with the view bounds,
 * indicating that the position is at least partially visible in the current view.
 *
 * @param viewbounds The rectangle representing the bounds of the current view.
 * @param position The rectangle representing the position to check for visibility.
 * @return true if the position is visible within the view bounds; false otherwise.
 */
constexpr inline bool is_visible_in_view( const sf::FloatRect &viewbounds, const sf::FloatRect &position )
{
  return viewbounds.findIntersection( position ).has_value();
}

/**
 * @brief Checks if a position is visible within a given view's bounds
 *
 * @param view The view to check against (in world coordinates)
 * @param position The position to test for visibility
 * @return true if the position's hitbox intersects with the view bounds
 */
constexpr inline bool is_visible_in_view( const sf::View &view, const sf::FloatRect &position )
{
  auto viewBounds = calculate_view_bounds( view );
  return viewBounds.findIntersection( position ).has_value();
}

//! @brief Get the Grid Position object
//!
//! @param entity The entity to get the grid position for.
//! @return std::optional<sf::Vector2i>
static std::optional<sf::Vector2i> getGridPosition( entt::registry &registry, entt::entity entity )
{
  auto pos = registry.try_get<Cmp::Position>( entity );
  if ( pos )
  {
    return std::optional<sf::Vector2i>{ { static_cast<int>( pos->position.x / Constants::kGridSquareSizePixels.x ),
                                          static_cast<int>( pos->position.y / Constants::kGridSquareSizePixels.y ) } };
  }
  else { SPDLOG_ERROR( "Entity {} does not have a Position component", static_cast<int>( entity ) ); }
  return std::nullopt;
}

//! @brief Get the Pixel Position object from an entity's Position component.
//!
//! @param entity The entity to get the pixel position for.
//! @return std::optional<sf::Vector2f>
static std::optional<sf::Vector2f> getPixelPosition( entt::registry &registry, entt::entity entity )
{
  auto pos = registry.try_get<Cmp::Position>( entity );
  if ( pos ) { return ( *pos ).position; }
  return std::nullopt;
}

namespace Maths
{

//! @brief Get the Manhattan Distance between two positions.
//! Creates a grid-like distance metric:
//! ┌────┬────┬────┐
//! │ 2  │ 1  │ 2  │  Sum of absolute differences
//! ├────┼────┼────┤  Moves only horizontal/vertical
//! │ 1  │ 0  │ 1  │  (Like a taxi in a city grid)
//! ├────┼────┼────┤
//! │ 2  │ 1  │ 2  │
//! └────┴────┴────┘
//! @note NPCs will be unable to "see" around corners with this distance metric.
//! @param posA The first position.
//! @param posB The second position.
//! @return unsigned int The Manhattan distance.
template <typename T>
constexpr inline T getManhattanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB )
{
  return std::abs( posA.x - posB.x ) + std::abs( posA.y - posB.y );
}

//! @brief Get the Chebyshev Distance between two positions.
//! Creates an equal-cost distance metric for all 8 directions:
//! ┌────┬────┬────┐
//! │ 1  │ 1  │ 1  │  Maximum of x or y distance
//! ├────┼────┼────┤  All 8 neighbors are distance 1
//! │ 1  │ 0  │ 1  │  (Like a chess king's move)
//! ├────┼────┼────┤
//! │ 1  │ 1  │ 1  │
//! └────┴────┴────┘
//! @note NPC pathfinding will pick randomly and appear to zig-zag
//! @param posA The first position.
//! @param posB The second position.
//! @return unsigned int The Chebyshev distance.
template <typename T>
constexpr inline T getChebyshevDistance( sf::Vector2<T> posA, sf::Vector2<T> posB )
{
  return std::max( std::abs( posA.x - posB.x ), std::abs( posA.y - posB.y ) );
}

//! @brief Get the Euclidean Distance between two positions.
//! Creates a straight-line distance metric:
//! ┌─────┬─────┬─────┐
//! │ 1.4 │ 1.0 │ 1.4 │  Straight-line distance
//! ├─────┼─────┼─────┤  Diagonal = √2 ≈ 1.414
//! │ 1.0 │ 0.0 │ 1.0 │  (Standard geometric distance)
//! ├─────┼─────┼─────┤
//! │ 1.4 │ 1.0 │ 1.4 │
//! └─────┴─────┴─────┘
//! @note NPCs pathfinding will be able to navigate around obstacles
//! @param posA The first position.
//! @param posB The second position.
//! @return unsigned int The Euclidean distance.
template <typename T>
constexpr inline T getEuclideanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB )
{
  T dx = posA.x - posB.x;
  T dy = posA.y - posB.y;
  return static_cast<T>( std::sqrt( dx * dx + dy * dy ) );
}

} // namespace Maths

} // namespace ProceduralMaze::Utils

#endif // SRC_UTILS_UTILS_HPP__