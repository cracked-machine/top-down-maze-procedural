#ifndef SRC_UTILS_MATHS_HPP__
#define SRC_UTILS_MATHS_HPP__

#include <Components/Position.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>

namespace ProceduralMaze::Utils::Maths
{

struct DistanceVector2fComparator
{
  bool operator()( const std::pair<float, sf::Vector2f> &a, const std::pair<float, sf::Vector2f> &b ) const
  {
    return a.first > b.first; // For min-heap (smallest distance first)
  }
};

struct DistancePositionComparator
{
  bool operator()( const std::pair<float, Cmp::Position> &a, const std::pair<float, Cmp::Position> &b ) const
  {
    return a.first > b.first; // For min-heap (smallest distance first)
  }
};

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
constexpr inline T getManhattanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB );

extern template int getManhattanDistance<int>( sf::Vector2i, sf::Vector2i );
extern template float getManhattanDistance<float>( sf::Vector2f, sf::Vector2f );

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
constexpr inline T getChebyshevDistance( sf::Vector2<T> posA, sf::Vector2<T> posB );

extern template int getChebyshevDistance<int>( sf::Vector2<int>, sf::Vector2<int> );
extern template float getChebyshevDistance<float>( sf::Vector2<float>, sf::Vector2<float> );

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
constexpr inline T getEuclideanDistance( sf::Vector2<T> posA, sf::Vector2<T> posB );

extern template int getEuclideanDistance<int>( sf::Vector2<int>, sf::Vector2<int> );
extern template float getEuclideanDistance<float>( sf::Vector2<float>, sf::Vector2<float> );

//! @brief Takes a floating-point angle and "wraps" it so that the result is always in the range [0, 2π).
//! @details 1. get the remainder after dividing by a full circle 2. If the result is negative, add 2π to bring it into the positive range.
//! @param angle +/- radians
//! @return float normalised positive-only radians
float normalizeAngle( float angle );

//! @brief Replaces SFML Vector2::normalized.
//! @param v
//! @return std::optional<sf::Vector2f> Zero vector returns std::nullopt instead of using assert
std::optional<sf::Vector2f> normalized( sf::Vector2f v );

//! @brief Replaces SFML Vector2::angle.
//! @param v
//! @return std::optional<sf::Angle> Zero vector returns std::nullopt instead of using assert
[[nodiscard]] inline std::optional<sf::Angle> angle( sf::Vector2f v );

//! @brief Create a thick line rect object
//! @example `m_window.draw( Utils::Maths::thick_line_rect( source_pos, corner, color, thickness ) );`
//! @param start
//! @param end
//! @param color
//! @param thickness
//! @return sf::RectangleShape
sf::RectangleShape thick_line_rect( sf::Vector2f start, sf::Vector2f end, sf::Color color, float thickness );

//! @brief Create a thick line quad object
//! @example `m_window.draw( Utils::Maths::thick_line_quad(quad.data(), quad.size(), sf::PrimitiveType::TriangleStrip) );`
//! @param start
//! @param end
//! @param color
//! @param thickness
//! @return std::array<sf::Vertex, 4>
std::array<sf::Vertex, 4> thick_line_quad( sf::Vector2f start, sf::Vector2f end, sf::Color color, float thickness );

//! @brief Convert the value to a percent based on the max value as 100%
//! @param max_value
//! @param convert
//! @return uint8_t
uint8_t to_percent( float max_value, uint8_t convert );

} // namespace ProceduralMaze::Utils::Maths

#endif // SRC_UTILS_MATHS_HPP__