#ifndef SRC_COMPONENTS_PARTICLE_SHOCKWAVE_HPP__
#define SRC_COMPONENTS_PARTICLE_SHOCKWAVE_HPP__

#include <Components/Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
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
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SHOCKWAVE_HPP__
