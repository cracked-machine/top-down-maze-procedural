#include <Components/Particle/SmokeWisp.hpp>

#include <numbers>
#include <random>

namespace ProceduralMaze::Cmp::Particle
{

void SmokeWispParticle::emit( sf::Vector2f emitter, sf::Time lifetime )
{
  // create random number generator
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  m_phase = std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> )( rng );
  m_wave_time = 0.f;

  m_velocity = sf::Vector2f( 0.f, -props.speed );
  m_lifetime = sf::milliseconds( std::uniform_int_distribution( 0, lifetime.asMilliseconds() )( rng ) );
  m_vertex.position = emitter;
};

SmokeWisp::SmokeWisp( sf::Vector2f emitter_pos )
    : ParticleSpriteBase( 100, sf::seconds( 0.3 ), emitter_pos ) {};

void SmokeWisp::simulate( sf::Time dt )
{
  const float frequency = 2.f;

  m_wave_time += dt.asSeconds();
  const float wave_x = (float)dt.asMilliseconds() * std::sin( ( 2.f * std::numbers::pi_v<float> * frequency * m_wave_time ) );

  for ( auto &p : m_particles_list )
  {
    p.m_lifetime -= dt;
    if ( p.m_lifetime <= sf::Time::Zero ) p.do_emit( m_emitter, m_lifetime );

    const sf::Vector2f velocity{ wave_x, p.m_velocity.y };
    p.m_vertex.position += velocity * dt.asSeconds();

    p.m_vertex.color.a = 255;
    p.m_vertex.color.r = 255;
    p.m_vertex.color.g = 0;
    p.m_vertex.color.b = 255;
  }
}

} // namespace ProceduralMaze::Cmp::Particle