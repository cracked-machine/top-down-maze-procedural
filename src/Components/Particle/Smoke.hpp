#ifndef SRC_CMP_PARTICLE_SMOKEWISP_HPP_
#define SRC_CMP_PARTICLE_SMOKEWISP_HPP_

#include <Particle/ParticleSpriteBase.hpp>

namespace ProceduralMaze::Cmp::Particle
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
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override;
};
} // namespace detail

//! @brief
class Smoke : public ParticleSpriteBase<detail::SmokeParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  Smoke( size_t count );
  void simulate( sf::Time dt ) override;

  float m_wave_time{ 0.f };   // single shared wave time for all particles
  float m_elapsed{ 0.f };     // tracks total time elapsed
  float m_rise_speed{ 20.f }; // pixels per second upward
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_SMOKEWISP_HPP_