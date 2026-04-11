#ifndef SRC_CMP_PARTICLE_SHOCKWAVE_HPP_
#define SRC_CMP_PARTICLE_SHOCKWAVE_HPP_

#include <Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace ProceduralMaze::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct ShockWaveParticle : public Cmp::Particle::ParticleBase
{
  struct Properties
  {
    float max_speed{ 100.f };
    sf::Angle max_angle{ sf::degrees( 360.f ) };
  } props;

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override;
};
} // namespace detail

//! @brief
class ShockWave : public ParticleSpriteBase<detail::ShockWaveParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ShockWave( sf::Vector2f emitter_pos, size_t generations );

  void simulate( sf::Time dt ) override;
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_SHOCKWAVE_HPP_