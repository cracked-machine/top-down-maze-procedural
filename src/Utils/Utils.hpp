#ifndef SRC_UTILS_UTILS_HPP__
#define SRC_UTILS_UTILS_HPP__

#include <SFML/Graphics/Rect.hpp>

#include <Systems/BaseSystem.hpp>

#include <cmath>

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
  float grid_size = Sys::BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
  sf::Vector2f snapped_pos{ std::round( position.position.x / Sys::BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                            std::round( position.position.y / Sys::BaseSystem::kGridSquareSizePixels.y ) * grid_size };

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
  float grid_size = Sys::BaseSystem::kGridSquareSizePixels.x; // Assuming square grid
  sf::Vector2f snapped_pos{ std::round( position.x / Sys::BaseSystem::kGridSquareSizePixels.x ) * grid_size,
                            std::round( position.y / Sys::BaseSystem::kGridSquareSizePixels.y ) * grid_size };

  return snapped_pos;
}

} // namespace ProceduralMaze::Utils

#endif // SRC_UTILS_UTILS_HPP__