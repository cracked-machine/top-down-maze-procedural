#ifndef SRC_SYSTEM_PARTICLESPRITEBASE_HPP_
#define SRC_SYSTEM_PARTICLESPRITEBASE_HPP_

#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp
{

//! @brief Interface that all particle types must implement
class IParticle
{
public:
  //! @brief Implements the emission stage of a particle.
  //         E.g. intialise vertex, velocity and lifetime particle members.
  //!        Should be called by the ParticleSpriteBase<T>::simulate implementation.
  //! @param emitter
  virtual void do_emit( sf::Vector2f emitter ) = 0;
  virtual ~IParticle() = default;

protected:
private:
  virtual void emit( sf::Vector2f emitter ) = 0;
  virtual void idle( sf::Vector2f emitter ) = 0;
};

//! @brief Enforces that TParticle inherits from IParticle
template <typename TParticle>
concept ParticleConcept = std::derived_from<TParticle, IParticle> && requires( TParticle p ) {
  { p.vertex } -> std::same_as<sf::Vertex &>;
  { p.velocity } -> std::same_as<sf::Vector2f &>;
  { p.lifetime } -> std::same_as<sf::Time &>;
};

struct ParticleBase : public Cmp::IParticle
{
  sf::Vertex vertex;
  sf::Vector2f velocity;
  sf::Time lifetime;
  void do_emit( [[maybe_unused]] sf::Vector2f emitter ) final
  {
    if ( m_particle_active ) { emit( emitter ); }
    else { idle( emitter ); }
  }

  //! @brief Disables IParticle::emit if false
  bool m_particle_active{ true };

private:
  void emit( sf::Vector2f emitter ) override = 0;
  void idle( [[maybe_unused]] sf::Vector2f emitter ) override { vertex.color.a = 0; }
};

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

  virtual void stop() = 0;
  virtual void restart() = 0;
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
      : m_particles( count ),
        m_lifetime( lifetime ),
        m_emitter( emitter_pos )
  {
    SPDLOG_INFO( "Created {} particles in sprite", count );
  }

  //! @brief Replaces the default translation function with a world -> screen translation function
  //! @param window
  //! @param world_view
  void set_emitter( sf::Vector2f position ) override { m_emitter = position; }

  // ParticleSpriteBase() = default;
  ~ParticleSpriteBase() {}

  void set_view_transform( const sf::RenderWindow &window, const sf::View &world_view ) override
  {
    m_world_to_screen = [&window, world_view]( sf::Vector2f world_pos ) -> sf::Vector2f
    { return sf::Vector2f( window.mapCoordsToPixel( world_pos, world_view ) ); };
  }

  void stop() override
  {
    if ( not m_sprite_active ) return;

    size_t dead_count = 0;
    for ( auto &p : m_particles )
    {
      p.m_particle_active = false;
      if ( p.lifetime <= sf::Time::Zero ) { dead_count++; }
    }
    SPDLOG_INFO( "Paritcles dead: {}", dead_count );
    if ( dead_count == m_particles.size() )
    {
      m_sprite_active = false;
      SPDLOG_INFO( "Stopping ParticleSprite" );
    }
  }

  void restart() override
  {
    if ( m_sprite_active ) return;
    for ( auto &p : m_particles )
    {
      p.m_particle_active = true;
    }
    m_sprite_active = true;
    SPDLOG_INFO( "Restarting ParticleSprite" );
  }

  bool is_active() override { return m_sprite_active; }

  //! @brief The list of particles in this sprite
  std::vector<TParticle> m_particles;

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
} // namespace ProceduralMaze::Cmp

#endif // SRC_SYSTEM_PARTICLESPRITEBASE_HPP_