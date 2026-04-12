#ifndef SRC_SYSTEM_PARTICLESPRITEBASE_HPP_
#define SRC_SYSTEM_PARTICLESPRITEBASE_HPP_

#include <Particle/ParticleConcepts.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <random>

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
  virtual void do_emit() = 0;
  virtual size_t generations() = 0;

  virtual void set_speed_dist( std::uniform_real_distribution<float> speed_dist ) = 0;
  virtual void set_angle_dist( std::uniform_real_distribution<float> angle_dist ) = 0;
  virtual void set_phase_dist( std::uniform_real_distribution<float> phase_dist ) = 0;
  virtual void set_freq_dist( std::uniform_real_distribution<float> freq_dist ) = 0;
  virtual void set_lifetime_dist( std::uniform_int_distribution<int> lifetime_dist ) = 0;
  virtual void set_emitter_position( sf::Vector2f emitter_position ) = 0;

private:
  // See ParticleBase for docstrings
  virtual void emit() = 0;
  virtual void idle() = 0;
};

struct ParticleBase : public Cmp::Particle::IParticle
{
  void do_emit() final
  {
    if ( m_particle_active )
    {
      m_vertex.position = m_emitter_position;
      static std::random_device rd;
      static std::mt19937 rng( rd() );
      m_lifetime = sf::milliseconds( m_lifetime_dist( rng ) );
      emit();
      m_generation++;
    }
    else { idle(); }
  }

  //! @brief Disables IParticle::emit if false
  bool m_particle_active{ true };
  void set_speed_dist( std::uniform_real_distribution<float> speed_dist ) override { m_speed_dist = speed_dist; }
  void set_angle_dist( std::uniform_real_distribution<float> angle_dist ) override { m_angle_dist = angle_dist; }
  void set_phase_dist( std::uniform_real_distribution<float> phase_dist ) override { m_phase_dist = phase_dist; }
  void set_freq_dist( std::uniform_real_distribution<float> freq_dist ) override { m_freq_dist = freq_dist; }
  void set_emitter_position( sf::Vector2f emitter_position ) override { m_emitter_position = emitter_position; }
  void set_lifetime_dist( std::uniform_int_distribution<int> lifetime_dist ) override { m_lifetime_dist = lifetime_dist; }

  sf::Vertex m_vertex;
  sf::Vector2f m_velocity;
  sf::Time m_lifetime;
  sf::Vector2f m_emitter_position;

  size_t generations() override { return m_generation; }
  size_t m_generation{ 0 };

protected:
  std::uniform_real_distribution<float> m_speed_dist;
  std::uniform_real_distribution<float> m_angle_dist;
  std::uniform_real_distribution<float> m_phase_dist;
  std::uniform_real_distribution<float> m_freq_dist;
  std::uniform_int_distribution<int> m_lifetime_dist;

private:
  //! @brief Run when the particle is enabled and expired. Derived class of ParticleBase must implement it.
  //! @param emitter
  //! @param lifetime
  void emit() override = 0;

  //! @brief Run when the particle is disabled and expired
  //! @param emitter
  void idle() override {}
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
  virtual void set_view_transform( const sf::RenderWindow &, const sf::View & ) = 0;
  virtual void stop() = 0;
  virtual void restart() = 0;
  virtual void prune_inactive_expired_particles() = 0;
  virtual void check_particle_collision( const sf::FloatRect &target ) = 0;
  virtual bool is_active() = 0;
  virtual void deactivate_extinct_particles() = 0;
  virtual void set_tag( const std::string &tag ) = 0;
  virtual std::string get_tag() const = 0;
  virtual void set_generations( size_t gen ) = 0;
  virtual size_t get_generations() = 0;

  virtual void set_speed( std::uniform_real_distribution<float> speed_dist ) = 0;
  virtual void set_speed( float speed ) = 0;

  virtual void set_angle( std::uniform_real_distribution<float> angle_dist ) = 0;
  virtual void set_angle( float angle ) = 0;

  virtual void set_phase( std::uniform_real_distribution<float> phase_dist ) = 0;
  virtual void set_phase( float phase ) = 0;

  virtual void set_freq( std::uniform_real_distribution<float> freq_dist ) = 0;
  virtual void set_freq( float freq ) = 0;

  virtual void set_emitter_position( sf::Vector2f emitter_position ) = 0;

  virtual void set_lifetime_ms( sf::Time lifetime ) = 0;
  virtual void set_lifetime_ms( std::uniform_int_distribution<int> life_dist ) = 0;
};

//! @brief Defines the particle sprite base class template. This renders a list of TParticle vertices.
//! @tparam TParticle The particle object type.
//! @note The derived class must implement the update function and
//        provide its own IParticle implementation that satisfies ParticleConcept.
template <ParticleConcept TParticle>
class ParticleSpriteBase : public IParticleSprite
{
public:
  ParticleSpriteBase() = delete;

  //! @brief Construct a new Particle Sprite Base object
  //! @param count Number of particles in this sprite
  //! @param lifetime The lifetime of the particles in this sprite
  //! @param emitter_pos The initial position of the emitter for this sprite
  explicit ParticleSpriteBase( size_t count )
      : m_max_particles( count ),
        m_particles_list( count )

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
      p.set_speed_dist( m_speed_dist );
      p.set_angle_dist( m_angle_dist );
      p.set_phase_dist( m_phase_dist );
      p.set_freq_dist( m_freq_dist );
      p.set_emitter_position( m_emitter_position );
      p.set_lifetime_dist( m_lifetime_dist );
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
  sf::Time m_lifetime{ sf::Time::Zero };

  void set_tag( const std::string &tag ) override { m_tag = tag; }
  std::string get_tag() const override { return m_tag; }

  void set_generations( size_t gen ) override { m_max_generations = gen; }
  size_t get_generations() override { return m_max_generations; }

  void set_speed( std::uniform_real_distribution<float> speed_dist ) override
  {
    m_speed_dist = speed_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_speed_dist( m_speed_dist );
    }
  }

  void set_speed( float speed ) override
  {
    m_speed_dist = std::uniform_real_distribution<float>( speed, speed );
    for ( auto &p : m_particles_list )
    {
      p.set_speed_dist( m_speed_dist );
    }
  }

  void set_angle( std::uniform_real_distribution<float> angle_dist ) override
  {
    m_angle_dist = angle_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_angle_dist( m_angle_dist );
    }
  }

  void set_angle( float angle ) override
  {
    m_angle_dist = std::uniform_real_distribution<float>( angle, angle );
    for ( auto &p : m_particles_list )
    {
      p.set_angle_dist( m_angle_dist );
    }
  }

  void set_phase( std::uniform_real_distribution<float> phase_dist ) override
  {
    m_phase_dist = phase_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_phase_dist( m_phase_dist );
    }
  }

  void set_phase( float phase ) override
  {
    m_phase_dist = std::uniform_real_distribution<float>( phase, phase );
    for ( auto &p : m_particles_list )
    {
      p.set_phase_dist( m_phase_dist );
    }
  }

  void set_freq( std::uniform_real_distribution<float> freq_dist ) override
  {
    m_freq_dist = freq_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_freq_dist( m_freq_dist );
    }
  }

  void set_freq( float freq ) override
  {
    m_freq_dist = std::uniform_real_distribution<float>( freq, freq );
    for ( auto &p : m_particles_list )
    {
      p.set_freq_dist( m_freq_dist );
    }
  }

  //! @brief
  //! @param position
  void set_emitter_position( sf::Vector2f emitter_position ) override
  {
    m_emitter_position = emitter_position;
    for ( auto &p : m_particles_list )
    {
      p.set_emitter_position( emitter_position );
    }
  }

  void set_lifetime_ms( std::uniform_int_distribution<int> lifetime_dist ) override
  {
    m_lifetime_dist = lifetime_dist;
    m_max_lifetime = sf::milliseconds( lifetime_dist.max() );
    static std::random_device rd;
    static std::mt19937 rng( rd() );
    m_lifetime = sf::milliseconds( m_lifetime_dist( rng ) );
    for ( auto &p : m_particles_list )
    {
      p.set_lifetime_dist( lifetime_dist );
    }
  }
  void set_lifetime_ms( sf::Time lifetime ) override
  {
    m_lifetime = lifetime;
    m_max_lifetime = lifetime;
    m_lifetime_dist = std::uniform_int_distribution<int>( lifetime.asMilliseconds(), lifetime.asMilliseconds() );
    for ( auto &p : m_particles_list )
    {
      p.set_lifetime_dist( m_lifetime_dist );
    }
  }

protected:
  //! @brief Default translation function is a noop. See set_view_transform()
  std::function<sf::Vector2f( sf::Vector2f )> m_world_to_screen = []( sf::Vector2f p ) { return p; };

  //! @brief Disables IParticleSprite::simulate() if false
  bool m_sprite_active{ true };
  sf::Time m_max_lifetime;

private:
  size_t m_max_generations{ 0 };
  std::uniform_real_distribution<float> m_speed_dist{ 0.f, 1.f };
  std::uniform_real_distribution<float> m_angle_dist{ 0.f, 360.f };
  std::uniform_real_distribution<float> m_phase_dist{ 0.f, 1.f };
  std::uniform_real_distribution<float> m_freq_dist{ 0.f, 1.f };
  std::uniform_int_distribution<int> m_lifetime_dist{ 0, 1 };
  std::string m_tag;
  //! @brief The emitter position
  sf::Vector2f m_emitter_position{ 0, 0 };
};
} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_SYSTEM_PARTICLESPRITEBASE_HPP_