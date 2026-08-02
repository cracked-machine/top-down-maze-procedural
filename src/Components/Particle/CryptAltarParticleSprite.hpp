#ifndef SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__
#define SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__

#include <Components/Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct CryptAltarParticle : public Cmp::Particle::ParticleBase
{

private:
  void emit() override;
};
} // namespace detail

//! @brief
class CryptAltarParticleSprite : public ParticleSpriteBase<detail::CryptAltarParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  CryptAltarParticleSprite( size_t count );

  void simulate( sf::Time dt ) override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__
