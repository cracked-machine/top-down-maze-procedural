#ifndef SRC_COMPONENTS_PARTICLE_SMOKE_HPP__
#define SRC_COMPONENTS_PARTICLE_SMOKE_HPP__

#include <Components/Particle/SpriteBase.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct SmokeParticle : public Cmp::Particle::ParticleBase
{
  // independent per particle
  float m_wave_time{ 0.f };
  float m_phase{ 0.f };
  float m_frequency{ 0.5f };

private:
  void emit() override;
};
} // namespace detail

//! @brief
class Smoke : public SpriteBase<detail::SmokeParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  Smoke( size_t count );
  void simulate( sf::Time dt ) override;

  float m_wave_time{ 0.f };   // single shared wave time for all particles
  float m_elapsed{ 0.f };     // tracks total time elapsed
  float m_rise_speed{ 20.f }; // pixels per second upward
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SMOKE_HPP__
