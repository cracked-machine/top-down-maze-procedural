#ifndef SRC_COMPONENTS_PARTICLE_SPRITEOWNER_HPP__
#define SRC_COMPONENTS_PARTICLE_SPRITEOWNER_HPP__

#include <Components/Particle/SpriteBase.hpp>

#include <memory>
#include <utility>

namespace Game::Cmp::Particle
{

//! @brief  This wraps SpriteBase<IParticle> so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ParticleSystem::find can retrieve SpriteBase<IParticle> via the specified `tag`
struct SpriteOwner
{
  //! @brief The owned particle sprite implementation.
  std::unique_ptr<Cmp::Particle::IParticleSprite> sprite;

  //! @brief Construct a new Particle Sprite Owner object
  //! @param sprite
  explicit SpriteOwner( std::unique_ptr<Cmp::Particle::IParticleSprite> sprite )
      : sprite( std::move( sprite ) )
  {
  }
};

} // namespace Game::Cmp::Particle

#endif // SRC_COMPONENTS_PARTICLE_SPRITEOWNER_HPP__
