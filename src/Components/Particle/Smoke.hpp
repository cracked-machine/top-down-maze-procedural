#ifndef SRC_COMPONENTS_PARTICLE_SMOKE_HPP__
#define SRC_COMPONENTS_PARTICLE_SMOKE_HPP__

#include <Components/Particle/SpriteBase.hpp>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
//! @brief Individual particle for the smoke effect. Rises and sways, sparsely visible and fading with lifetime.
struct SmokeParticle : public Cmp::Particle::ParticleBase
{
  //! @brief Elapsed time (seconds) since this particle's own wave motion last reset, independent of the other particles.
  float m_wave_time{ 0.f };
  //! @brief This particle's own randomly-assigned sway wave phase.
  float m_phase{ 0.f };
  //! @brief This particle's own randomly-assigned sway wave frequency.
  float m_frequency{ 0.5f };

private:
  //! @brief Rolls a new phase/frequency and launches the particle straight upward on (re)emission.
  void emit() override;
};
} // namespace detail

//! @brief Particle sprite for a smoke effect: particles rise, sway, and sparsely fade out over their lifetime.
class Smoke : public SpriteBase<detail::SmokeParticle>
{
public:
  //! @brief Construct a new Smoke object
  //! @param count Number of particles in this sprite
  Smoke( size_t count );
  //! @brief Advances the smoke simulation by one frame (sway and density-based fade).
  //! @param dt Time elapsed since the last frame.
  void simulate( sf::Time dt ) override;

  //! @brief Single wave time shared across all particles (currently unused by simulate(), which tracks per-particle wave time instead).
  float m_wave_time{ 0.f };
  //! @brief Tracks total time elapsed since this sprite started simulating.
  float m_elapsed{ 0.f };
  //! @brief Upward emitter rise speed in pixels per second (currently unused by simulate()).
  float m_rise_speed{ 20.f };
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SMOKE_HPP__
