#ifndef SRC_COMPONENTS_PARTICLE_PLAYERHEALINGPARTICLESPRITE_HPP__
#define SRC_COMPONENTS_PARTICLE_PLAYERHEALINGPARTICLESPRITE_HPP__

#include <Components/Particle/SpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct PlayerHealingParticle : public Cmp::Particle::ParticleBase
{

private:
  void emit() override;
};
} // namespace detail

//! @brief
class PlayerHealingParticleSprite : public SpriteBase<detail::PlayerHealingParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  PlayerHealingParticleSprite( size_t count );

  void simulate( sf::Time dt ) override;
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_PLAYERHEALINGPARTICLESPRITE_HPP__
