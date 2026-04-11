#ifndef SRC_CMP_PARTICLE_SMOKEWISP_HPP_
#define SRC_CMP_PARTICLE_SMOKEWISP_HPP_

#include <Particle/ParticleSpriteBase.hpp>

namespace ProceduralMaze::Cmp::Particle
{

struct SmokeWispParticle : public Cmp::Particle::ParticleBase
{
  float m_phase{ 0.f };     // unique phase offset per particle
  float m_wave_time{ 0.f }; // accumulates real time for the sine wave

  struct Properties
  {
    float speed{ 50.f };
    sf::Angle max_angle{ sf::degrees( 270.f ) };
  } props;

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override;
};

//! @brief
class SmokeWisp : public ParticleSpriteBase<SmokeWispParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  SmokeWisp( sf::Vector2f emitter_pos );
  void simulate( sf::Time dt ) override;

  float m_wave_time{ 0.f }; // single shared wave time for all particles
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_SMOKEWISP_HPP_