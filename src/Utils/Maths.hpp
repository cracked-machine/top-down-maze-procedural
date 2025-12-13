#ifndef SRC_UTILS_MATHS_HPP__
#define SRC_UTILS_MATHS_HPP__

#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>

namespace ProceduralMaze::Utils::Maths
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
} // namespace ProceduralMaze::Utils::Maths

#endif // SRC_UTILS_MATHS_HPP__