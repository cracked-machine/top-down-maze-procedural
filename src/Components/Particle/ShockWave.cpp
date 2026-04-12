#include <Particle/ShockWave.hpp>

#include <random>

namespace ProceduralMaze::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void ShockWaveParticle::emit( sf::Time lifetime )
{
  // create random number generator
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  const sf::Angle angle = sf::degrees( m_angle_dist( rng ) );
  const float speed = m_speed_dist( rng );
  m_velocity = sf::Vector2f( speed, angle );

  m_lifetime = sf::milliseconds( std::uniform_int_distribution( 0, lifetime.asMilliseconds() )( rng ) );
};
} // namespace detail

ShockWave::ShockWave( size_t count )
    : ParticleSpriteBase( count ) {};

void ShockWave::simulate( sf::Time dt )
{

  for ( auto &p : m_particles_list )
  {
    // update the particle lifetime
    p.m_lifetime -= dt;

    // if the particle is dead, respawn it
    if ( p.m_lifetime <= sf::Time::Zero ) { p.do_emit( m_lifetime ); }

    // update the position of the corresponding vertex
    p.m_vertex.position += p.m_velocity * dt.asSeconds();

    p.m_vertex.color.a = 128;
    p.m_vertex.color.r = 255;
    p.m_vertex.color.g = 0;
    p.m_vertex.color.b = 255;
  }
}

} // namespace ProceduralMaze::Cmp::Particle