#ifndef SRC_COMPONENTS_PARTICLE_SPRITEBASE_HPP__
#define SRC_COMPONENTS_PARTICLE_SPRITEBASE_HPP__

#include <Components/Particle/Concepts.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <random>

namespace Game::Cmp::Particle
{

enum class ViewType { SCREEN, WORLD };

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

  virtual void set_speed( std::uniform_real_distribution<float> speed ) = 0;
  virtual void set_angle( std::uniform_real_distribution<float> angle ) = 0;
  virtual void set_phase( std::uniform_real_distribution<float> phase ) = 0;
  virtual void set_freq( std::uniform_real_distribution<float> freq ) = 0;
  virtual void set_lifetime( std::uniform_int_distribution<int> lifetime ) = 0;
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
      m_lifetime = sf::milliseconds( m_lifetime_range( rng ) );
      emit();
      m_generation++;
    }
    else { idle(); }
  }

  //! @brief Disables IParticle::emit if false
  bool m_particle_active{ true };
  void set_speed( std::uniform_real_distribution<float> speed ) override { m_speed_range = speed; }
  void set_angle( std::uniform_real_distribution<float> angle ) override { m_angle_range = angle; }
  void set_phase( std::uniform_real_distribution<float> phase ) override { m_phase_range = phase; }
  void set_freq( std::uniform_real_distribution<float> freq ) override { m_freq_range = freq; }
  void set_emitter_position( sf::Vector2f emitter_position ) override { m_emitter_position = emitter_position; }
  void set_lifetime( std::uniform_int_distribution<int> lifetime ) override { m_lifetime_range = lifetime; }

  sf::Vertex m_vertex;
  sf::Vector2f m_velocity;
  sf::Time m_lifetime;
  sf::Vector2f m_emitter_position;

  size_t generations() override { return m_generation; }
  size_t m_generation{ 0 };

protected:
  std::uniform_real_distribution<float> m_speed_range;
  std::uniform_real_distribution<float> m_angle_range;
  std::uniform_real_distribution<float> m_phase_range;
  std::uniform_real_distribution<float> m_freq_range;
  std::uniform_int_distribution<int> m_lifetime_range;

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

  // Check SpriteBase for docstrings
  virtual void set_view_transform( const sf::RenderWindow &, const sf::View & ) = 0;
  virtual void reset_view_transform() = 0;
  virtual void stop() = 0;
  virtual void clear() = 0;
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
  virtual sf::Vector2f get_emitter_position() = 0;

  virtual void set_lifetime_ms( sf::Time lifetime ) = 0;
  virtual void set_lifetime_ms( std::uniform_int_distribution<int> life_dist ) = 0;

  virtual void set_view_type( ViewType v ) = 0;
  virtual ViewType get_view_type() = 0;
};

//! @brief Defines the particle sprite base class template. This renders a list of TParticle vertices.
//! @tparam TParticle The particle object type.
//! @note The derived class must implement the update function and
//        provide its own IParticle implementation that satisfies ParticleConcept.
template <ParticleConcept TParticle>
class SpriteBase : public IParticleSprite
{
public:
  SpriteBase() = delete;

  //! @brief Construct a new Particle Sprite Base object
  //! @param count Number of particles in this sprite
  //! @param lifetime The lifetime of the particles in this sprite
  //! @param emitter_pos The initial position of the emitter for this sprite
  explicit SpriteBase( size_t count )
      : m_max_particles( count ),
        m_particles_list( count )

  {
    SPDLOG_DEBUG( "Created {} particles in sprite", count );
  }

  ~SpriteBase() {}

  //! @brief Replaces the default translation function with a world -> screen translation function
  //! @param window
  //! @param world_view
  void set_view_transform( const sf::RenderWindow &window, const sf::View &world_view ) override
  {
    m_world_to_screen = [&window, world_view]( sf::Vector2f world_pos ) -> sf::Vector2f
    { return sf::Vector2f( window.mapCoordsToPixel( world_pos, world_view ) ); };
  }

  void reset_view_transform() override
  {
    m_world_to_screen = []( sf::Vector2f p ) { return p; };
  }

  //! @brief Disables the particles for this ParticleSprite (they will continue simulating until their lifetimes expire)
  void stop() override
  {
    SPDLOG_DEBUG( "ParticleSprite Stop Signal Received" );

    for ( auto &p : m_particles_list )
    {
      // prevent emit() reseting the particle lifetime
      p.m_particle_active = false;
    }
    SPDLOG_DEBUG( "Particles have been disabled" );
  }

  void clear() override
  {
    for ( auto &p : m_particles_list )
    {
      p.m_particle_active = false;
      p.m_lifetime = sf::Time::Zero;
    }
    prune_inactive_expired_particles();
  }

  //! @brief Remove inactive and dead particles for this ParticleSprite
  void prune_inactive_expired_particles() override
  {

    std::erase_if( m_particles_list, []( const TParticle &p ) { return not p.m_particle_active and p.m_lifetime <= sf::Time::Zero; } );

    if ( m_particles_list.empty() )
    {
      SPDLOG_DEBUG( "Particles have been deleted" );
      // prevent ParticleSystem from calling this->simulate()
      m_sprite_active = false;
      SPDLOG_DEBUG( "ParticleSprite has stopped" );
    }
  }

  //! @brief Creates a new particle list, enables the particles, resets the generation counter and restarts the simulation.
  void restart() override
  {
    if ( m_sprite_active ) return;
    SPDLOG_DEBUG( "Restarting ParticleSprite" );

    m_particles_list = std::vector<TParticle>( m_max_particles );
    for ( auto &p : m_particles_list )
    {
      p.set_speed( m_cached_speed_range );
      p.set_angle( m_cached_angle_range );
      p.set_phase( m_cached_phase_range );
      p.set_freq( m_cached_freq_range );
      p.set_emitter_position( m_emitter_position );
      p.set_lifetime( m_cached_lifetime_range );
      p.m_particle_active = true;
    }
    m_sprite_active = true;
    SPDLOG_DEBUG( "ParticleSprite is running." );
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
    constexpr int kSides = 4;
    for ( const auto &p : m_particles_list )
    {

      // map world -> screen
      const auto pos = m_world_to_screen( p.m_vertex.position );

      const auto colour = p.m_vertex.color;

      // unit circle divided into wedges
      constexpr float kAngleStep = 2.f * std::numbers::pi_v<float> / static_cast<float>( kSides );

      for ( int i = 0; i < kSides; ++i )
      {
        // the two outside facing (polar) angles for the current wedge
        const float a0 = kAngleStep * static_cast<float>( i );
        const float a1 = kAngleStep * static_cast<float>( i + 1 );

        // the centre position is always the middle of the polygon
        verts.push_back( { pos, colour } );
        // get the vertex positions by converting the polar coords to cartesian coords (euler's formula).
        verts.push_back( { pos + sf::Vector2f( kSize, sf::radians( a0 ) ), colour } );
        verts.push_back( { pos + sf::Vector2f( kSize, sf::radians( a1 ) ), colour } );
      }
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
    m_cached_speed_range = speed_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_speed( m_cached_speed_range );
    }
  }

  void set_speed( float speed ) override
  {
    m_cached_speed_range = std::uniform_real_distribution<float>( speed, speed );
    for ( auto &p : m_particles_list )
    {
      p.set_speed( m_cached_speed_range );
    }
  }

  void set_angle( std::uniform_real_distribution<float> angle_dist ) override
  {
    m_cached_angle_range = angle_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_angle( m_cached_angle_range );
    }
  }

  void set_angle( float angle ) override
  {
    m_cached_angle_range = std::uniform_real_distribution<float>( angle, angle );
    for ( auto &p : m_particles_list )
    {
      p.set_angle( m_cached_angle_range );
    }
  }

  void set_phase( std::uniform_real_distribution<float> phase_dist ) override
  {
    m_cached_phase_range = phase_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_phase( m_cached_phase_range );
    }
  }

  void set_phase( float phase ) override
  {
    m_cached_phase_range = std::uniform_real_distribution<float>( phase, phase );
    for ( auto &p : m_particles_list )
    {
      p.set_phase( m_cached_phase_range );
    }
  }

  void set_freq( std::uniform_real_distribution<float> freq_dist ) override
  {
    m_cached_freq_range = freq_dist;
    for ( auto &p : m_particles_list )
    {
      p.set_freq( m_cached_freq_range );
    }
  }

  void set_freq( float freq ) override
  {
    m_cached_freq_range = std::uniform_real_distribution<float>( freq, freq );
    for ( auto &p : m_particles_list )
    {
      p.set_freq( m_cached_freq_range );
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

  sf::Vector2f get_emitter_position() override { return m_emitter_position; }

  void set_lifetime_ms( std::uniform_int_distribution<int> lifetime_dist ) override
  {
    m_cached_lifetime_range = lifetime_dist;
    m_max_lifetime = sf::milliseconds( lifetime_dist.max() );
    static std::random_device rd;
    static std::mt19937 rng( rd() );
    m_lifetime = sf::milliseconds( m_cached_lifetime_range( rng ) );
    for ( auto &p : m_particles_list )
    {
      p.set_lifetime( lifetime_dist );
    }
  }
  void set_lifetime_ms( sf::Time lifetime ) override
  {
    m_lifetime = lifetime;
    m_max_lifetime = lifetime;
    m_cached_lifetime_range = std::uniform_int_distribution<int>( lifetime.asMilliseconds(), lifetime.asMilliseconds() );
    for ( auto &p : m_particles_list )
    {
      p.set_lifetime( m_cached_lifetime_range );
    }
  }

  void set_view_type( ViewType v ) override { m_view_type = v; };
  ViewType get_view_type() override { return m_view_type; };

protected:
  //! @brief Default translation function is a noop. See set_view_transform()
  std::function<sf::Vector2f( sf::Vector2f )> m_world_to_screen = []( sf::Vector2f p ) { return p; };

  //! @brief Disables IParticleSprite::simulate() if false
  bool m_sprite_active{ true };
  sf::Time m_max_lifetime;

private:
  size_t m_max_generations{ 0 };

  //! @brief Particle velocity range
  //! @note  Cache the init value so we can reset it later
  std::uniform_real_distribution<float> m_cached_speed_range{ 0.f, 1.f };

  //! @brief Particle angle range
  //! @note  Cache the init value so we can reset it later
  std::uniform_real_distribution<float> m_cached_angle_range{ 0.f, 360.f };

  //! @brief Particle phase range
  //! @note  Cache the init value so we can reset it later
  std::uniform_real_distribution<float> m_cached_phase_range{ 0.f, 1.f };

  //! @brief Particle frequency range
  //! @note  Cache the init value so we can reset it later
  std::uniform_real_distribution<float> m_cached_freq_range{ 0.f, 1.f };

  //! @brief Particle lifetime range
  //! @note  Cache the init value so we can reset it later
  std::uniform_int_distribution<int> m_cached_lifetime_range{ 0, 1 };

  //! @brief convenience identifier for searching the registry
  std::string m_tag;
  //! @brief The emitter position
  sf::Vector2f m_emitter_position{ 0, 0 };
  //! @brief Support world vs screen rendering
  ViewType m_view_type{ ViewType::WORLD };
};
} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SPRITEBASE_HPP__
