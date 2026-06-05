#include <Particle/ShockWave.hpp>

#include <random>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void ShockWaveParticle::emit()
{
  // create random number generator
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  const sf::Angle angle = sf::degrees( m_angle_range( rng ) );
  const float speed = m_speed_range( rng );
  m_velocity = sf::Vector2f( speed, angle );
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
    if ( p.m_lifetime <= sf::Time::Zero ) { p.do_emit(); }

    // update the position of the corresponding vertex
    p.m_vertex.position += p.m_velocity * dt.asSeconds();

    p.m_vertex.color.a = 128;
    p.m_vertex.color.r = 254;
    p.m_vertex.color.g = 64;
    p.m_vertex.color.b = 64;
  }
}

//! @brief Allows this sprite to be passed into RenderWindow.draw()
//! @note  Overriding it here so we can draw larger particles than the default.
//! @param target
//! @param states
void ShockWave::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  states.texture = nullptr;
  states.blendMode = sf::BlendAlpha;

  // project out just the vertices for drawing
  std::vector<sf::Vertex> verts;
  verts.reserve( m_particles_list.size() );
  SPDLOG_DEBUG( "Drawing {} particles", m_particles.size() );

  constexpr float kSize = 8.f;
  constexpr int kSides = 5; // pentagon
  for ( const auto &p : m_particles_list )
  {

    // map world -> screen
    const auto pos = m_world_to_screen( p.m_vertex.position );

    const auto col = p.m_vertex.color;

    // Triangle fan from center — one triangle per pentagon edge
    for ( int i = 0; i < kSides; ++i )
    {
      const float a0 = ( -std::numbers::pi_v<float> / 2.f ) + ( ( 2.f * std::numbers::pi_v<float> * i ) / kSides );
      const float a1 = ( -std::numbers::pi_v<float> / 2.f ) + ( ( 2.f * std::numbers::pi_v<float> * ( i + 1 ) ) / kSides );

      verts.push_back( { pos, col } );
      verts.push_back( { { pos.x + ( kSize * std::cos( a0 ) ), pos.y + ( kSize * std::sin( a0 ) ) }, col } );
      verts.push_back( { { pos.x + ( kSize * std::cos( a1 ) ), pos.y + ( kSize * std::sin( a1 ) ) }, col } );
    }
  }

  target.draw( verts.data(), verts.size(), sf::PrimitiveType::Triangles, states );
}

} // namespace Game::Cmp::Particle