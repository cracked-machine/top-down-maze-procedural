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

private:
  void emit() override;
};
} // namespace detail

//! @brief
class ShockWave : public ParticleSpriteBase<detail::ShockWaveParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ShockWave( size_t count );

  void simulate( sf::Time dt ) override;
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_SHOCKWAVE_HPP_