#ifndef SRC_SYSTEM_PARTICLESPRITETEST_HPP_
#define SRC_SYSTEM_PARTICLESPRITETEST_HPP_

#include <Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>
#include <random>

namespace ProceduralMaze::Cmp
{

struct ParticleTest : public Cmp::ParticleBase
{
private:
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
      : ParticleSpriteBase( 1000, sf::seconds( 3 ), emitter_pos )
  {

    // emit the particles early to prevent delayed start
    // while we wait for particles to die first time
    for ( auto &p : m_particles )
    {
      p.do_emit( m_emitter );
    }
  };

  void simulate( sf::Time dt ) override
  {
    for ( auto &p : m_particles )
    {
      // update the particle lifetime
      p.lifetime -= dt;

      // if the particle is dead, respawn it
      if ( p.lifetime <= sf::Time::Zero ) p.do_emit( m_emitter );

      // update the position of the corresponding vertex
      p.vertex.position += p.velocity * dt.asSeconds();
      p.vertex.color.a = 128;

      p.vertex.color.r = 255;
      p.vertex.color.g = 0;
      p.vertex.color.b = 255;
    }
  }

  //! @brief Allows this sprite to be passed into RenderWindow.draw()
  //! @param target
  //! @param states
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {

    // states.transform *= getTransform();
    states.texture = nullptr;
    states.blendMode = sf::BlendAlpha;

    // project out just the vertices for drawing
    std::vector<sf::Vertex> verts;
    verts.reserve( m_particles.size() );
    SPDLOG_DEBUG( "Drawing {} particles", m_particles.size() );

    constexpr float kSize = 2.f;
    for ( const auto &p : m_particles )
    {
      // verts.push_back( p.vertex );

      const auto pos = m_world_to_screen( p.vertex.position ); // map world -> screen
      const auto col = p.vertex.color;

      // triangle 1
      verts.push_back( { { pos.x - kSize, pos.y - kSize }, col } );
      verts.push_back( { { pos.x + kSize, pos.y - kSize }, col } );
      verts.push_back( { { pos.x + kSize, pos.y + kSize }, col } );
      // triangle 2
      verts.push_back( { { pos.x - kSize, pos.y - kSize }, col } );
      verts.push_back( { { pos.x + kSize, pos.y + kSize }, col } );
      verts.push_back( { { pos.x - kSize, pos.y + kSize }, col } );
    }

    target.draw( verts.data(), verts.size(), sf::PrimitiveType::Triangles, states );
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_SYSTEM_PARTICLESPRITETEST_HPP_