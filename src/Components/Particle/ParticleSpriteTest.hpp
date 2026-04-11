#ifndef SRC_CMP_PARTICLE_PARTICLESPRITETEST_HPP_
#define SRC_CMP_PARTICLE_PARTICLESPRITETEST_HPP_

#include <Particle/ParticleSpriteBase.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <Systems/ParticleSystem.hpp>

namespace ProceduralMaze::Cmp::Particle
{

struct ParticleTest : public Cmp::Particle::ParticleBase
{
  struct Properties
  {
    float max_speed{ 100.f };
    sf::Angle max_angle{ sf::degrees( 360.f ) };
  } props;

private:
  void emit( sf::Vector2f emitter, sf::Time lifetime ) override;
};

//! @brief
class ParticleSpriteTest : public ParticleSpriteBase<ParticleTest>
{
public:
  //! @brief Construct a new Particle Sprite Test object
  ParticleSpriteTest( sf::Vector2f emitter_pos );

  void simulate( sf::Time dt ) override;
};

} // namespace ProceduralMaze::Cmp::Particle

#endif // SRC_CMP_PARTICLE_PARTICLESPRITETEST_HPP_