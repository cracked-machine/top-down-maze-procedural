#ifndef SRC_COMPONENTS_PARTICLE_OBSTACLEDIGPARTICLESPRITE_HPP__
#define SRC_COMPONENTS_PARTICLE_OBSTACLEDIGPARTICLESPRITE_HPP__

#include <Components/Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

#include <array>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
struct ObstacleDigParticle : public Cmp::Particle::ParticleBase
{
  //! @brief Add some variation to colour of each particle
  int m_color_variation = 0;

  //! @brief This particle's own randomly-assigned lifetime at spawn, captured so speed falloff can
  //!        ease out relative to it rather than the sprite-wide max lifetime
  sf::Time m_initial_lifetime;

  //! @brief Number of vertices in this particle's polygon (excluding the centre)
  static constexpr int kVertexCount = 5;

  //! @brief Per-particle jittered vertex angles, randomized on emit() to give each particle an irregular polygon shape
  std::array<float, kVertexCount> m_vertex_angles{};

private:
  void emit() override;
};
} // namespace detail

//! @brief
class ObstacleDigParticleSprite : public ParticleSpriteBase<detail::ObstacleDigParticle>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ObstacleDigParticleSprite( size_t count );

  void simulate( sf::Time dt ) override;
  void draw( sf::RenderTarget &target, sf::RenderStates states ) const override;
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_OBSTACLEDIGPARTICLESPRITE_HPP__
