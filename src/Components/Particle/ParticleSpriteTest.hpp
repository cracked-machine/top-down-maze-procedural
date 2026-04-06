#ifndef SRC_SYSTEM_PARTICLESPRITETEST_HPP_
#define SRC_SYSTEM_PARTICLESPRITETEST_HPP_

#include <Systems/ParticleSystem.hpp>
#include <random>

namespace ProceduralMaze::Cmp
{

struct ParticleTest : public Cmp::IParticle
{
  sf::Vertex vertex;
  sf::Vector2f velocity;
  sf::Time lifetime;
  void emit( sf::Vector2f emitter ) override
  {
    // create random number generator
    static std::random_device rd;
    static std::mt19937 rng( rd() );

    // give a random velocity and lifetime to the particle
    const sf::Angle angle = sf::degrees( std::uniform_real_distribution( 0.f, 360.f )( rng ) );
    const float speed = std::uniform_real_distribution( 50.f, 100.f )( rng );
    velocity = sf::Vector2f( speed, angle );
    lifetime = sf::milliseconds( std::uniform_int_distribution( 1000, 3000 )( rng ) );

    // reset the position of the corresponding vertex
    vertex.position = emitter;
  };
};

//! @brief
class ParticleSpriteTest : public Cmp::ParticleSpriteBase<ParticleTest>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ParticleSpriteTest( sf::Vector2f emitter_pos )
      : ParticleSpriteBase( 1000, sf::seconds( 3 ), emitter_pos ) {};

  void simulate( sf::Time dt ) override
  {
    SPDLOG_DEBUG( "Updating {} particles", m_particles.size() );
    for ( auto &p : m_particles )
    {
      // update the particle lifetime
      p.lifetime -= dt;

      // if the particle is dead, respawn it
      if ( p.lifetime <= sf::Time::Zero ) p.emit( m_emitter );

      // update the position of the corresponding vertex
      p.vertex.position += p.velocity * dt.asSeconds();

      // update the alpha (transparency) of the particle according to its lifetime
      float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
      p.vertex.color.a = static_cast<std::uint8_t>( ratio * 255 );
    }
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_SYSTEM_PARTICLESPRITETEST_HPP_