#ifndef SRC_COMPONENTS_PARTICLE_SPARKLES_HPP__
#define SRC_COMPONENTS_PARTICLE_SPARKLES_HPP__

#include <Components/Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct ParticleSparkles : public Cmp::Particle::ParticleBase
{

private:
  void emit() override;
};
} // namespace detail

//! @brief
class Sparkles : public ParticleSpriteBase<detail::ParticleSparkles>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  Sparkles( size_t count );

  void simulate( sf::Time dt ) override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SPARKLES_HPP__
