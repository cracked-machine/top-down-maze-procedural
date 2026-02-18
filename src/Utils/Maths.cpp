
#include <Components/Position.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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

template int getManhattanDistance<int>( sf::Vector2i, sf::Vector2i );
template float getManhattanDistance<float>( sf::Vector2f, sf::Vector2f );

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

template int getChebyshevDistance<int>( sf::Vector2<int>, sf::Vector2<int> );
template float getChebyshevDistance<float>( sf::Vector2<float>, sf::Vector2<float> );

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

template int getEuclideanDistance<int>( sf::Vector2<int>, sf::Vector2<int> );
template float getEuclideanDistance<float>( sf::Vector2<float>, sf::Vector2<float> );

float normalizeAngle( float angle )
{
  angle = std::fmod( angle, 2.0f * std::numbers::pi );
  return angle < 0 ? angle + 2.0f * std::numbers::pi : angle;
}

//! @brief Create a thick line rect object
//! @example `m_window.draw( Utils::Maths::thick_line_rect( source_pos, corner, color, thickness ) );`
//! @param start
//! @param end
//! @param color
//! @param thickness
//! @return sf::RectangleShape
sf::RectangleShape thick_line_rect( sf::Vector2f start, sf::Vector2f end, sf::Color color, float thickness )
{
  sf::Vector2f direction = end - start;
  float length = direction.length();
  sf::Angle angle = direction.angle();

  sf::RectangleShape line( { length, thickness } );
  line.setPosition( start );
  line.setOrigin( { 0.f, thickness / 2.f } ); // center vertically
  line.setRotation( angle );
  line.setFillColor( color );
  return line;
};

//! @brief Create a thick line quad object
//! @example `m_window.draw( Utils::Maths::thick_line_quad(quad.data(), quad.size(), sf::PrimitiveType::TriangleStrip) );`
//! @param start
//! @param end
//! @param color
//! @param thickness
//! @return std::array<sf::Vertex, 4>
std::array<sf::Vertex, 4> thick_line_quad( sf::Vector2f start, sf::Vector2f end, sf::Color color, float thickness )
{
  sf::Vector2f direction = ( end - start ).normalized();
  sf::Vector2f perpendicular{ -direction.y, direction.x };
  sf::Vector2f offset = perpendicular * ( thickness / 2.f );

  std::array<sf::Vertex, 4> quad = { sf::Vertex{ start - offset, color }, sf::Vertex{ start + offset, color }, sf::Vertex{ end - offset, color },
                                     sf::Vertex{ end + offset, color } };
  return quad;
};

uint8_t to_percent( float max_value, uint8_t convert )
{
  auto converted = std::round( ( max_value / 100 ) * convert );
  return static_cast<uint8_t>( converted );
}

} // namespace ProceduralMaze::Utils::Maths
