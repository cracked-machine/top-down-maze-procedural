#include <Components/Particle/Smoke.hpp>

#include <numbers>
#include <random>

namespace ProceduralMaze::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void SmokeParticle::emit( sf::Vector2f emitter, sf::Time lifetime )
{
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  m_wave_time = 0.f;
  m_phase = std::uniform_real_distribution( 0.f, 2.f * std::numbers::pi_v<float> )( rng );
  m_frequency = std::uniform_real_distribution( 0.3f, 0.8f )( rng );
  m_velocity = sf::Vector2f( 0.f, -props.speed );
  m_lifetime = sf::milliseconds( std::uniform_int_distribution( 0, lifetime.asMilliseconds() )( rng ) );
  m_vertex.position = emitter;
};
} // namespace detail

Smoke::Smoke( sf::Vector2f emitter_pos, size_t gen )
    : ParticleSpriteBase( 100, sf::seconds( 1 ), emitter_pos, gen ) {};

void Smoke::simulate( sf::Time dt )
{
  m_elapsed += dt.asSeconds();
  m_emitter.y -= m_rise_speed * dt.asSeconds(); // move emitter upward over time

  const float amplitude = 20.f;
  const sf::Color smoke_color{ 0, 0, 0 };
  constexpr float k_smoke_density = 0.5f; // 10% of particles visible in smoke phase

  static std::mt19937 rng( std::random_device{}() );
  static std::uniform_real_distribution<float> density_dist( 0.f, 1.f );

  for ( auto &p : m_particles_list )
  {
    p.m_lifetime -= dt;
    p.m_wave_time += dt.asSeconds();
    const float ratio = p.m_lifetime.asSeconds() / m_lifetime.asSeconds();
    if ( p.m_lifetime <= sf::Time::Zero ) p.do_emit( { m_emitter.x, m_emitter.y }, m_lifetime );

    const float wave_x = ( ratio > 0.8f ) ? amplitude * std::sin( ( 2.f * std::numbers::pi_v<float> * p.m_frequency * p.m_wave_time ) + p.m_phase )
                                          : 0.f;

    p.m_vertex.position += sf::Vector2f{ wave_x, p.m_velocity.y } * dt.asSeconds();

    const bool visible = density_dist( rng ) < k_smoke_density;
    p.m_vertex.color = smoke_color;
    p.m_vertex.color.a = visible ? static_cast<std::uint8_t>( ratio * 255 ) : 0;
  }
}

} // namespace ProceduralMaze::Cmp::Particle