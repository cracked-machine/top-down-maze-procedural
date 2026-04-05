#ifndef SRC_CMPS_LIGHTNINGSTRIKE_HPP_
#define SRC_CMPS_LIGHTNINGSTRIKE_HPP_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Time.hpp>
namespace ProceduralMaze::Cmp
{

struct LightningStrike
{

  struct AngleDeviations
  {
    float inner;
    float outer;
  } m_deviations;

  LightningStrike( sf::Vector2f start, sf::Vector2f end, Cmp::LightningStrike::AngleDeviations deviations, sf::Time duration )
  {
    sequence.push_back( { sf::Vertex( start ) } );
    sequence.push_back( { sf::Vertex( end ) } );
    m_deviations = deviations;
    this->duration = duration;
    timer.stop();
  }

  std::vector<std::vector<sf::Vertex>> sequence;

  sf::Color color;
  sf::Time duration{ sf::Time::Zero };
  sf::Clock timer;
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_LIGHTNINGSTRIKE_HPP_