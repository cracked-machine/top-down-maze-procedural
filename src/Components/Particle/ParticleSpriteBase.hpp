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
  virtual size_t generations() = 0;

private:
  // See ParticleBase for docstrings
  virtual void emit( sf::Vector2f emitter, sf::Time lifetime ) = 0;
  virtual void idle( sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) = 0;
};

struct ParticleBase : public Cmp::Particle::IParticle
{
  void do_emit( sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) final
  {
    if ( m_particle_active )
    {
      emit( emitter, lifetime );
      m_generation++;
    }
    else { idle( emitter, lifetime ); }
  }

  //! @brief Disables IParticle::emit if false
  bool m_particle_active{ true };

  sf::Vertex m_vertex;
  sf::Vector2f m_velocity;
  sf::Time m_lifetime;

  size_t generations() override { return m_generation; }
  size_t m_generation{ 0 };

private:
  //! @brief Run when the particle is enabled and expired. Derived class of ParticleBase must implement it.
  //! @param emitter
  //! @param lifetime
  //! @param props
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override = 0;

  //! @brief Run when the particle is disabled and expired
  //! @param emitter
  void idle( [[maybe_unused]] sf::Vector2f emitter, [[maybe_unused]] sf::Time lifetime ) override {}
};

// ============================================================
// IParticleSprite — particle sprite container contract
// ============================================================

//! @brief Non-template abstract base — allows ParticleSpriteOwner and find() to work without knowing TParticle
class IParticleSprite : public sf::Drawable, public sf::Transformable
{
public:
  virtual ~IParticleSprite() = default;

  //! @brief  Implements the simulation stage of all particles.
  //          E.g. modify the vertex and velocity members of each particle,
  //          and reset the particle if its lifetime has expired
  //! @param dt
  virtual void simulate( sf::Time dt ) = 0;

  // Check ParticleSpriteBase for docstrings
  virtual void set_emitter( sf::Vector2f position ) = 0;
  virtual void set_view_transform( const sf::RenderWindow &, const sf::View & ) = 0;
  virtual void stop() = 0;
  virtual void restart() = 0;
  virtual void prune_inactive_expired_particles() = 0;
  virtual void check_particle_collision( const sf::FloatRect &target ) = 0;
  virtual bool is_active() = 0;
  virtual void deactivate_extinct_particles() = 0;
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
  ParticleSpriteBase( size_t count, sf::Time lifetime, sf::Vector2f emitter_pos, size_t max_generations )
      : m_max_particles( count ),
        m_particles_list( count ),
        m_lifetime( lifetime ),
        m_emitter( emitter_pos ),
        m_max_generations( max_generations )

  {
    SPDLOG_INFO( "Created {} particles in sprite", count );
  }

  ~ParticleSpriteBase() {}

  //! @brief Replaces the default translation function with a world -> screen translation function
  //! @param window
  //! @param world_view
  void set_view_transform( const sf::RenderWindow &window, const sf::View &world_view ) override
  {
    m_world_to_screen = [&window, world_view]( sf::Vector2f world_pos ) -> sf::Vector2f
    { return sf::Vector2f( window.mapCoordsToPixel( world_pos, world_view ) ); };
  }

  //! @brief Disables the particles for this ParticleSprite (they will continue simulating until their lifetimes expire)
  void stop() override
  {
    SPDLOG_INFO( "ParticleSprite Stop Signal Received" );

    for ( auto &p : m_particles_list )
    {
      // prevent emit() reseting the particle lifetime
      p.m_particle_active = false;
    }
    SPDLOG_INFO( "Particles have been disabled" );
  }

  //! @brief Remove inactive and dead particles for this ParticleSprite
  void prune_inactive_expired_particles() override
  {
    for ( auto &p : m_particles_list )
    {
      std::erase_if( m_particles_list, []( const TParticle &p ) { return p.m_particle_active == false and p.m_lifetime <= sf::Time::Zero; } );

      if ( m_particles_list.empty() )
      {
        SPDLOG_INFO( "Particles have been deleted" );
        // prevent ParticleSystem from calling this->simulate()
        m_sprite_active = false;
        SPDLOG_INFO( "ParticleSprite has stopped" );
      }
    }
  }

  //! @brief Creates a new particle list, enables the particles, resets the generation counter and restarts the simulation.
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

  //! @brief Check if the Particles from this ParticleSprite colide with the target
  //! @param target Rectangle collision area
  void check_particle_collision( const sf::FloatRect &target ) override
  {
    //
    for ( auto &p : m_particles_list )
    {
      if ( not target.contains( p.m_vertex.position ) ) continue;
      p.m_lifetime = sf::Time::Zero;
    }
  }

  //! @brief Is simulation running for this ParticleSprite?
  //! @return true
  //! @return false
  bool is_active() override { return m_sprite_active; }

  //! @brief Allow access to the emmitter without casting to the concrete type
  //! @param position
  void set_emitter( sf::Vector2f position ) override { m_emitter = position; }

  //! @brief Increase the generation count when the ParticleSprite lifetime has expired
  void deactivate_extinct_particles() override
  {
    if ( m_max_generations == 0 ) return;

    bool all_particles_last_generation = true;
    for ( auto &p : m_particles_list )
    {
      if ( p.m_generation < m_max_generations ) { all_particles_last_generation = false; }
    }

    if ( all_particles_last_generation ) { stop(); }
  }

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

  size_t m_max_generations;
};
} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_SYSTEM_PARTICLESPRITEBASE_HPP_