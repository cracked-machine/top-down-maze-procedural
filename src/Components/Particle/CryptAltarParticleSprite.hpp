#ifndef SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__
#define SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__

#include <Components/Particle/SpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
//! @brief Individual particle for the crypt altar's sparkle effect.
struct CryptAltarParticle : public Cmp::Particle::ParticleBase
{

private:
  //! @brief Resets velocity/angle for the sparkle on (re)emission.
  void emit() override;
};
} // namespace detail

//! @brief Particle sprite for the visual effect played when a crypt altar is activated.
class CryptAltarParticleSprite : public SpriteBase<detail::CryptAltarParticle>
{
public:
  //! @brief Construct a new Crypt Altar Particle Sprite object
  //! @param count Number of particles in this sprite
  CryptAltarParticleSprite( size_t count );

  //! @brief Advances the sparkle simulation by one frame.
  //! @param dt Time elapsed since the last frame.
  void simulate( sf::Time dt ) override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_CRYPTALTARSPARKLES_HPP__
