#include <Components/Particle/Flame.hpp>

#include <numbers>
#include <random>

namespace ProceduralMaze::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void FlameParticle::emit()
{
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  m_wave_time = 0.f;

  m_phase = m_phase_dist( rng );
  m_frequency = m_freq_dist( rng );

  m_velocity = sf::Vector2f( 0.f, -m_speed_dist( rng ) );
};
} // namespace detail

Flame::Flame( size_t count )
    : ParticleSpriteBase( count ) {};

void Flame::simulate( sf::Time dt )
{
  const float amplitude = 20.f;

  const sf::Color start_flame_color{ 255, 255, 255 }; // white
  const sf::Color final_flame_color{ 255, 0, 0 };     // red
  const sf::Color smoke_color{ 0, 0, 0 };

  constexpr float k_flame_phase = 0.6f;   // first 50% of lifetime = flame
  constexpr float k_smoke_density = 0.1f; // 10% of particles visible in smoke phase

  static std::mt19937 rng( std::random_device{}() );
  static std::uniform_real_distribution<float> density_dist( 0.f, 1.f );

  for ( auto &p : m_particles_list )
  {
    p.m_lifetime -= dt;
    p.m_wave_time += dt.asSeconds();
    if ( p.m_lifetime <= sf::Time::Zero ) p.do_emit();

    const float ratio = p.m_lifetime.asSeconds() / m_lifetime.asSeconds();

    const float wave_x = ( ratio > 0.8f ) ? amplitude * std::sin( ( 2.f * std::numbers::pi_v<float> * p.m_frequency * p.m_wave_time ) + p.m_phase )
                                          : 0.f;

    p.m_vertex.position += sf::Vector2f{ wave_x, p.m_velocity.y } * dt.asSeconds();

    if ( ratio > k_flame_phase )
    {
      // flame phase — lerp white -> red, ratio 1.0 = white, k_flame_phase = red
      const float t = 1.f - ( ( ratio - k_flame_phase ) / ( 1.f - k_flame_phase ) );
      p.m_vertex.color.r = static_cast<std::uint8_t>(
          std::lerp( static_cast<float>( start_flame_color.r ), static_cast<float>( final_flame_color.r ), t ) );
      p.m_vertex.color.g = static_cast<std::uint8_t>(
          std::lerp( static_cast<float>( start_flame_color.g ), static_cast<float>( final_flame_color.g ), t ) );
      p.m_vertex.color.b = static_cast<std::uint8_t>(
          std::lerp( static_cast<float>( start_flame_color.b ), static_cast<float>( final_flame_color.b ), t ) );
      p.m_vertex.color.a = static_cast<std::uint8_t>( ratio * 255 );
    }
    else
    {
      // smoke phase — grey, 10% density, fade out
      const bool visible = density_dist( rng ) < k_smoke_density;
      p.m_vertex.color = smoke_color;
      p.m_vertex.color.a = visible ? static_cast<std::uint8_t>( ratio * 255 ) : 0;
    }
  }
}

} // namespace ProceduralMaze::Cmp::Particle