#ifndef SRC_SYSTEM_PARTICLESPRITETEST_HPP_
#define SRC_SYSTEM_PARTICLESPRITETEST_HPP_

#include <Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>
#include <random>

namespace ProceduralMaze::Cmp
{

struct ParticleTestProps
{
  float max_speed{ 100.f };
  sf::Angle max_angle{ sf::degrees( 360.f ) };
};

struct ParticleTest : public Cmp::ParticleBase<ParticleTestProps>
{

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime, const ParticleTestProps &props ) override
  {
    // create random number generator
    static std::random_device rd;
    static std::mt19937 rng( rd() );

    const sf::Angle angle = sf::degrees( std::uniform_real_distribution( 0.f, props.max_angle.asDegrees() )( rng ) );
    const float speed = std::uniform_real_distribution( 1.f, props.max_speed )( rng );
    m_velocity = sf::Vector2f( speed, angle );
    m_lifetime = sf::milliseconds( std::uniform_int_distribution( 0, lifetime.asMilliseconds() )( rng ) );
    m_vertex.position = emitter;
  };
};

//! @brief
class ParticleSpriteTest : public ParticleSpriteBase<ParticleTestProps, ParticleTest>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ParticleSpriteTest( sf::Vector2f emitter_pos )
      : ParticleSpriteBase( 1000, sf::seconds( 3 ), emitter_pos ) {};

  void simulate( sf::Time dt ) override
  {
    for ( auto &p : m_particles_list )
    {
      // update the particle lifetime
      p.m_lifetime -= dt;

      // if the particle is dead, respawn it
      if ( p.m_lifetime <= sf::Time::Zero ) p.do_emit( m_emitter, m_lifetime, m_props );

      // update the position of the corresponding vertex
      p.m_vertex.position += p.m_velocity * dt.asSeconds();

      p.m_vertex.color.a = 128;
      p.m_vertex.color.r = 255;
      p.m_vertex.color.g = 0;
      p.m_vertex.color.b = 255;
    }
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_SYSTEM_PARTICLESPRITETEST_HPP_