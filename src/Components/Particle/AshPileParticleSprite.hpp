#ifndef SRC_COMPONENTS_PARTICLE_ASHPILEPARTICLESPRITE_HPP__
#define SRC_COMPONENTS_PARTICLE_ASHPILEPARTICLESPRITE_HPP__

#include <Components/Particle/SpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>

#include <array>

namespace Game::Cmp::Particle
{

//! @brief Implementation detail — do not use externally
namespace detail
{
//! @brief Individual particle for the ash pile effect: falls from the pyramid's apex (the emitter)
//!        and settles at a random point inside the pyramid's triangular cross-section.
struct AshPileParticle : public Cmp::Particle::ParticleBase
{

  //! @brief Gate the emit function to only fire once per particle
  bool m_emitted_particle = false;

  //! @brief Add some variation to the ash colour of each particle
  int m_color_variation = 0;

private:
  //! @brief Rolls this particle's colour, start delay and fall speed, then pins its lifetime so the
  //!        base class never re-triggers this — it only ever falls once.
  void emit() override;
};
} // namespace detail

//! @brief Particle sprite for a small pyramid-shaped pile of ash. Particles fall from the apex (the
//! emitter), staggered over add_ashpile()'s `buildup_seconds` window, and fill the pyramid outward from
//! the centre pair of columns: each new particle walks outward from its starting centre column and
//! settles in whichever reachable column currently holds the fewest particles, so the pile fills level
//! by level in rings rather than spiking any single column to full height first or scattering thinly
//! across the whole width. The pyramid is kFallDistance (8px) tall and kBaseWidth (16px) wide, centred
//! on the emitter's x position.
class AshPileParticleSprite : public SpriteBase<detail::AshPileParticle>
{
public:
  //! @brief Construct a new Ash Pile Particle Sprite object
  AshPileParticleSprite();

  //! @brief Advances the ash pile simulation by one frame: once a particle's staggered start delay has
  //! elapsed, claims it a landing spot on the shared pile (on first arrival only), eases it from the
  //! apex towards that spot, then holds it there permanently.
  //! @param dt Time elapsed since the last frame.
  void simulate( sf::Time dt ) override;

private:
  static constexpr int kPyramidHeight = 8;
  static constexpr int kPyramidWidth = 16;
  static constexpr int kPyramidHalfWidth = kPyramidWidth / 2;

  //! @brief Total number of slots across all columns' ragged heights (1,2,...,8,8,...,2,1), i.e. the
  //!        pyramid's actual triangular capacity — smaller than kPyramidHeight * kPyramidWidth, which
  //!        is the bounding rectangle.
  static constexpr int kMaxPopulation = []
  {
    int total = 0;
    for ( int c = 0; c < kPyramidHeight; ++c )
      total += ( c + 1 );
    for ( int c = 0; c < kPyramidHeight; ++c )
      total += ( kPyramidHeight - c );
    return total;
  }();

  sf::Time m_emit_delay_accumulator = sf::Time::Zero;

  //! @brief number of pyramid particles currently added to the sprite
  int m_pyramid_population = 0;

  //! @brief Particle cols for the pyramid
  std::array<std::vector<bool>, kPyramidWidth> m_pyramid{};
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_ASHPILEPARTICLESPRITE_HPP__
