#ifndef SRC_SYSTEM_PARTICLESPRITEBASE_HPP_
#define SRC_SYSTEM_PARTICLESPRITEBASE_HPP_

#include <Particle/ParticleConcepts.hpp>

namespace ProceduralMaze::Cmp::Particle
{

// ============================================================
// IParticle — individual particle contract
// ============================================================

//! @brief Interface that all particle types must implement
class IParticle
{
public:
  virtual ~IParticle() = default;

  //! @brief API for emitting the particle. Should be called by `simulate()` function of derived class.
  //! @param emitter
  //! @param lifetime
  //! @param props
  virtual void do_emit( sf::Vector2f emitter, sf::Time lifetime ) = 0;

private:
  //! @brief Run when the particle is enabled and expired. Derived class of ParticleBase must implement it.
  //! @param emitter
  //! @param lifetime
  //! @param props
  virtual void emit( sf::Vector2f emitter, sf::Time lifetime ) = 0;

  //! @brief Run when the particle is disabled and expired
  //! @param emitter
  virtual void idle( sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) = 0;
};

struct ParticleBase : public Cmp::Particle::IParticle
{

  void do_emit( sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) final
  {
    if ( m_particle_active ) { emit( emitter, lifetime ); }
    else { idle( emitter, lifetime ); }
  }

  //! @brief Disables IParticle::emit if false
  bool m_particle_active{ true };

  sf::Vertex m_vertex;
  sf::Vector2f m_velocity;
  sf::Time m_lifetime;

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override = 0;
  void idle( [[maybe_unused]] sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) override {}
};

// ============================================================
// IParticleSprite — particle sprite container contract
// ============================================================

//! @brief Non-template abstract base — allows ParticleSpriteOwner and find() to work without knowing TParticle
class IParticleSprite : public sf::Drawable, public sf::Transformable
{
public:
  //! @brief  Implements the simulation stage of all particles.
  //          E.g. modify the vertex and velocity members of each particle,
  //          and reset the particle if its lifetime has expired
  //! @param dt
  virtual void simulate( sf::Time dt ) = 0;

  //! @brief Allow access to the emmitter without casting to the concrete type
  //! @param position
  virtual void set_emitter( sf::Vector2f position ) = 0;

  //! @brief Replaces the default translation function with a world -> screen translation function
  //! @param window
  //! @param world_view
  virtual void set_view_transform( const sf::RenderWindow &, const sf::View & ) = 0;
  virtual ~IParticleSprite() = default;

  //! @brief Signals that particles should expire, deletes the expired particles, then stops the simulation.
  virtual void stop() = 0;

  //! @brief Creates a new particle list, enables the particles and resumes the simulation.
  virtual void restart() = 0;

  virtual void check_collision( const sf::FloatRect &target ) = 0;

  //! @brief Is simulation running?
  //! @return true
  //! @return false
  virtual bool is_active() = 0;
};

//! @brief Defines the particle sprite base class template. This renders a list of TParticle vertices.
//! @tparam TParticle The particle object type.
//! @note The derived class must implement the update function and
//        provide its own IParticle implementation that satisfies ParticleConcept.
template <ParticleConcept TParticle>
class ParticleSpriteBase : public IParticleSprite
{
public:
  //! @brief Construct a new Particle Sprite Base object
  //! @param count Number of particles in this sprite
  //! @param lifetime The lifetime of the particles in this sprite
  //! @param emitter_pos The initial position of the emitter for this sprite
  ParticleSpriteBase( size_t count, sf::Time lifetime, sf::Vector2f emitter_pos )
      : m_max_particles( count ),
        m_particles_list( count ),
        m_lifetime( lifetime ),
        m_emitter( emitter_pos )
  {
    SPDLOG_INFO( "Created {} particles in sprite", count );
  }

  ~ParticleSpriteBase() {}

  void set_view_transform( const sf::RenderWindow &window, const sf::View &world_view ) override
  {
    m_world_to_screen = [&window, world_view]( sf::Vector2f world_pos ) -> sf::Vector2f
    { return sf::Vector2f( window.mapCoordsToPixel( world_pos, world_view ) ); };
  }

  void stop() override
  {
    if ( not m_sprite_active ) return;
    for ( auto &p : m_particles_list )
    {
      // prevent emit() reseting the particle lifetime
      p.m_particle_active = false;
    }

    // remove dead particles
    std::erase_if( m_particles_list, []( const TParticle &p ) { return p.m_lifetime <= sf::Time::Zero; } );

    if ( m_particles_list.empty() )
    {
      // signal ParticleSystem not to call simulate()
      m_sprite_active = false;
      SPDLOG_INFO( "Stopping ParticleSprite" );
    }
  }

  void restart() override
  {
    if ( m_sprite_active ) return;

    m_particles_list = std::vector<TParticle>( m_max_particles );
    for ( auto &p : m_particles_list )
    {
      p.m_particle_active = true;
    }
    m_sprite_active = true;

    SPDLOG_INFO( "Restarting ParticleSprite" );
  }

  void check_collision( const sf::FloatRect &target ) override
  {
    //
    for ( auto &p : m_particles_list )
    {
      if ( not target.contains( p.m_vertex.position ) ) continue;
      p.m_lifetime = sf::Time::Zero;
    }
  }

  bool is_active() override { return m_sprite_active; }
  void set_emitter( sf::Vector2f position ) override { m_emitter = position; }

  //! @brief Allows this sprite to be passed into RenderWindow.draw()
  //! @param target
  //! @param states
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override
  {
    states.texture = nullptr;
    states.blendMode = sf::BlendAlpha;

    // project out just the vertices for drawing
    std::vector<sf::Vertex> verts;
    verts.reserve( m_particles_list.size() );
    SPDLOG_DEBUG( "Drawing {} particles", m_particles.size() );

    constexpr float kSize = 2.f;
    for ( const auto &p : m_particles_list )
    {

      // map world -> screen
      const auto pos = m_world_to_screen( p.m_vertex.position );

      const auto col = p.m_vertex.color;

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

  //! @brief Max size of `m_particles_list`
  size_t m_max_particles;

  //! @brief The list of particles in this sprite
  std::vector<TParticle> m_particles_list;

  //! @brief The lifetime of the particles in this sprite
  sf::Time m_lifetime;

  //! @brief The emitter position
  sf::Vector2f m_emitter;

protected:
  //! @brief Default translation function is a noop. See set_view_transform()
  std::function<sf::Vector2f( sf::Vector2f )> m_world_to_screen = []( sf::Vector2f p ) { return p; };

  //! @brief Disables IParticleSprite::simulate() if false
  bool m_sprite_active{ true };
};
} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_SYSTEM_PARTICLESPRITEBASE_HPP_