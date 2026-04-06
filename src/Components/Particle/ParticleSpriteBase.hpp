#ifndef SRC_SYSTEM_PARTICLESPRITEBASE_HPP_
#define SRC_SYSTEM_PARTICLESPRITEBASE_HPP_

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
  virtual void emit( sf::Vector2f emitter ) = 0;
  virtual ~IParticle() = default;
};

//! @brief Enforces that TParticle inherits from IParticle
template <typename TParticle>
concept ParticleConcept = std::derived_from<TParticle, IParticle> && requires( TParticle p ) {
  { p.vertex } -> std::same_as<sf::Vertex &>;
  { p.velocity } -> std::same_as<sf::Vector2f &>;
  { p.lifetime } -> std::same_as<sf::Time &>;
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
  virtual ~IParticleSprite() = default;
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

  void set_emitter( sf::Vector2f position ) override { m_emitter = position; }

  ParticleSpriteBase() = default;
  ~ParticleSpriteBase() {}

  //! @brief The list of particles in this sprite
  std::vector<TParticle> m_particles;

  //! @brief The lifetime of the particles in this sprite
  sf::Time m_lifetime;

  //! @brief The emitter position
  sf::Vector2f m_emitter;
};
} // namespace ProceduralMaze::Cmp

#endif // SRC_SYSTEM_PARTICLESPRITEBASE_HPP_