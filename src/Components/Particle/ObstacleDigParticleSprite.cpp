#include <Components/Particle/ObstacleDigParticleSprite.hpp>

#include <Components/Random.hpp>
#include <random>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
void ObstacleDigParticle::emit()
{
  m_color_variation = Cmp::RandomInt( 0, 50 ).gen();

  // do_emit() already assigned this particle's own randomly-chosen lifetime before calling emit();
  // capture it so speed falloff can ease out relative to THIS particle's lifespan, not the sprite-wide max
  m_initial_lifetime = m_lifetime;

  // create random number generator
  static std::random_device rd;
  static std::mt19937 rng( rd() );

  // jitter each vertex around its evenly-spaced base angle to make an irregular polygon.
  // the jitter is bounded well within half a step so vertices can't cross over and cause
  // the polygon to self-intersect
  static constexpr float kBaseStep = 2.f * std::numbers::pi_v<float> / static_cast<float>( kVertexCount );
  static std::uniform_real_distribution<float> jitter_dist( -kBaseStep * 0.4f, kBaseStep * 0.4f );

  const sf::Angle angle = sf::degrees( m_angle_range( rng ) );
  const float speed = m_speed_range( rng );
  m_vertex.position = m_emitter_position;
  m_velocity = sf::Vector2f( speed, angle );

  for ( int i = 0; i < kVertexCount; ++i )
    m_vertex_angles[i] = ( kBaseStep * static_cast<float>( i ) ) + jitter_dist( rng );
};
} // namespace detail

ObstacleDigParticleSprite::ObstacleDigParticleSprite( size_t count )
    : ParticleSpriteBase( count ) {};

void ObstacleDigParticleSprite::simulate( sf::Time dt )
{
  for ( auto &p : m_particles_list )
  {
    // update the particle lifetime
    p.m_lifetime -= dt;

    // if the particle is dead, respawn it
    if ( p.m_lifetime <= sf::Time::Zero ) p.do_emit();

    // cube the lifetime ratio so speed drops off sharply early on and eases out near zero,
    // rather than bleeding off at a constant rate. Ease out against this particle's OWN initial
    // lifetime, not the sprite-wide max, otherwise short-lived particles start already decelerated
    const float initial_lifetime = p.m_initial_lifetime.asSeconds();
    const float lifetime_ratio = initial_lifetime > 0.f ? p.m_lifetime.asSeconds() / initial_lifetime : 0.f;
    const float speed_falloff = lifetime_ratio * lifetime_ratio * lifetime_ratio * lifetime_ratio;

    // update the position of the corresponding vertex
    p.m_vertex.position += p.m_velocity * speed_falloff * dt.asSeconds();

    p.m_vertex.color.a = 255;
    p.m_vertex.color.r = 60 + p.m_color_variation;
    p.m_vertex.color.g = 80 + p.m_color_variation;
    p.m_vertex.color.b = 130 + p.m_color_variation;
  }
}

//! @brief Allows this sprite to be passed into RenderWindow.draw()
//! @param target
//! @param states
void ObstacleDigParticleSprite::draw( sf::RenderTarget &target, sf::RenderStates states ) const
{
  states.texture = nullptr;
  states.blendMode = sf::BlendAlpha;

  // project out just the vertices for drawing
  std::vector<sf::Vertex> verts;
  std::vector<sf::Vertex> outline_verts;
  verts.reserve( m_particles_list.size() );
  SPDLOG_DEBUG( "Drawing {} particles", m_particles.size() );

  constexpr float kSize = 7.f;
  constexpr sf::Color kOutlineColour = sf::Color( 0, 0, 0, 128 );
  for ( const auto &p : m_particles_list )
  {

    // map world -> screen
    const auto pos = m_world_to_screen( p.m_vertex.position );

    const auto colour = p.m_vertex.color;

    // irregular polygon: each particle's vertex angles were jittered once on emit()
    constexpr size_t kVertexCount = detail::ObstacleDigParticle::kVertexCount;

    for ( size_t i = 0; i < kVertexCount; ++i )
    {
      // the two outside facing (polar) angles for the current wedge
      const float a0 = p.m_vertex_angles[i];
      const float a1 = p.m_vertex_angles[( i + 1 ) % kVertexCount];

      // get the vertex positions by converting the polar coords to cartesian coords (euler's formula).
      const auto v0 = pos + sf::Vector2f( kSize, sf::radians( a0 ) );
      const auto v1 = pos + sf::Vector2f( kSize, sf::radians( a1 ) );

      // the centre position is always the middle of the polygon
      verts.push_back( { pos, colour } );
      verts.push_back( { v0, colour } );
      verts.push_back( { v1, colour } );

      // this wedge's outer edge is one segment of the polygon's boundary
      outline_verts.push_back( { v0, kOutlineColour } );
      outline_verts.push_back( { v1, kOutlineColour } );
    }
  }

  target.draw( verts.data(), verts.size(), sf::PrimitiveType::Triangles, states );
  target.draw( outline_verts.data(), outline_verts.size(), sf::PrimitiveType::Lines, states );
}

} // namespace Game::Cmp::Particle