#ifndef SRC_COMPONENTS_FRACTALCURVE_HPP__
#define SRC_COMPONENTS_FRACTALCURVE_HPP__

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Time.hpp>

namespace Game::Cmp
{

//! @brief A pair of vertices that are continually split into more pairs of vertices
//!        with a perpendicular offset at each vertex.
struct FractalCurve
{
  //! @brief Max random perpendicular offset (degrees) applied when subdividing a segment
  struct AngleDeviations
  {
    //! @brief Deviation applied to the first (main-line) vertex of each subdivided segment
    float inner;
    //! @brief Deviation applied to the remaining forked/branch vertices of each subdivided segment
    float outer;
  } m_deviations;

  FractalCurve( sf::Vector2f start, sf::Vector2f end, AngleDeviations deviations, sf::Time duration )
  {
    sequence.push_back( { sf::Vertex( start ) } );
    sequence.push_back( { sf::Vertex( end ) } );
    m_deviations = deviations;
    this->duration = duration;
    timer.stop();
  }

  //! @brief Rows of vertices produced by successive subdivisions; each row is one generation of split points
  std::vector<std::vector<sf::Vertex>> sequence;

  //! @brief Colour the curve is rendered with
  sf::Color color;

  //! @brief Lifetime. Destruction logic should be managed externally.
  sf::Time duration{ sf::Time::Zero };

  //! @brief Tracks elapsed time since the curve was created, checked against `duration`
  sf::Clock timer;
};

//! @brief Type of FractalCurve used for LightningStrikes
struct LightningStrike : public FractalCurve
{
  LightningStrike( sf::Vector2f start, sf::Vector2f end, AngleDeviations deviations, sf::Time duration )
      : FractalCurve( start, end, deviations, duration )
  {
  }
};

//! @brief Type of FractalCurve used for ObstacleCracks
struct ObstacleCrack : public FractalCurve
{
  ObstacleCrack( sf::Vector2f start, sf::Vector2f end, AngleDeviations deviations, sf::Time duration )
      : FractalCurve( start, end, deviations, duration )
  {
  }
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_FRACTALCURVE_HPP__
