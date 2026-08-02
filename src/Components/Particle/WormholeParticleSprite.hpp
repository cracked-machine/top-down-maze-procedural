#ifndef SRC_COMPONENTS_PARTICLE_WORMHOLEPARTICLESPRITE_HPP__
#define SRC_COMPONENTS_PARTICLE_WORMHOLEPARTICLESPRITE_HPP__

#include <Components/Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct WormholeParticle : public Cmp::Particle::ParticleBase
{
  float m_wave_time{ 0.f };
  float m_phase{ 0.f };
  float m_frequency{ 0.5f };
  sf::Vector2f m_perpendicular{ 0.f, 0.f };

private:
  void emit() override;
};
} // namespace detail

//! @brief
class WormholeParticleSprite : public ParticleSpriteBase<detail::WormholeParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  WormholeParticleSprite( size_t count );

  void simulate( sf::Time dt ) override;
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_WORMHOLEPARTICLESPRITE_HPP__
