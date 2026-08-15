#ifndef SRC_COMPONENTS_PARTICLE_FLAME_HPP__
#define SRC_COMPONENTS_PARTICLE_FLAME_HPP__

#include <Components/Particle/SpriteBase.hpp>

namespace Game::Cmp::Particle
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

private:
  void emit() override;
};
} // namespace detail

//! @brief
class Flame : public SpriteBase<detail::FlameParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  Flame( size_t count );
  void simulate( sf::Time dt ) override;

  float m_wave_time{ 0.f };                   // single shared wave time for all particles
  sf::Color m_final_flame_color{ 255, 0, 0 }; // red
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_FLAME_HPP__
