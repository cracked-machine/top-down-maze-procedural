#ifndef SRC_CMP_PARTICLE_FLAME_HPP_
#define SRC_CMP_PARTICLE_FLAME_HPP_

#include <Particle/ParticleSpriteBase.hpp>

namespace ProceduralMaze::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct FlameParticle : public Cmp::Particle::ParticleBase
{
  // independent per particle
  float m_wave_time{ 0.f };
  float m_phase{ 0.f };
  float m_frequency{ 0.5f };

  struct Properties
  {
    float speed{ 40.f };
    sf::Angle max_angle{ sf::degrees( 270.f ) };
  } props;

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override;
};
} // namespace detail

//! @brief
class Flame : public ParticleSpriteBase<detail::FlameParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  Flame( size_t count );
  void simulate( sf::Time dt ) override;

  float m_wave_time{ 0.f }; // single shared wave time for all particles
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_FLAME_HPP_