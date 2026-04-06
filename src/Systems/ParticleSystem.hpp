#ifndef SRC_SYSTEMS_PARTICLESYSTEM_HPP_
#define SRC_SYSTEMS_PARTICLESYSTEM_HPP_

#include <Particle/ParticleSpriteBase.hpp>
#include <Systems/BaseSystem.hpp>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

// 1. Emission Stage:   - IParticle::emit()
// 2. Simulation Stage  - IParticleSprite::simulate()
// 3. Rendering Stage   - RenderGameSystem::render_particle_sprites()

// IParticle                            contract: emit(), vertex, velocity, lifetime
// IParticleSprite                      contract: simulate(), set_emitter(), draw()
//   └── ParticleSpriteBase<IParticle>  template: owns vector<T>, implements draw()

// ParticleSpriteOwner                  entt component: tag + unique_ptr<IParticleSprite>

// ParticleSystem                       add(), update(), find() — knows nothing about concrete types
// RenderGameSystem                     iterates ParticleSpriteOwner, calls draw() — knows nothing about concrete types

//! @brief  This wraps ParticleSpriteBase<IParticle> so it can be emplaced/retrieved with the Entt registry as a single type.
//!         ParticleSystem::find can retrieve ParticleSpriteBase<IParticle> via the specified `tag`
struct ParticleSpriteOwner
{
  std::string tag; // identifies the sprite e.g. "player_dust", "explosion"
  std::unique_ptr<Cmp::IParticleSprite> sprite;

  explicit ParticleSpriteOwner( std::string tag, std::unique_ptr<Cmp::IParticleSprite> sprite )
      : tag( std::move( tag ) ),
        sprite( std::move( sprite ) )
  {
  }
};

//! @brief Core system for adding and updating ParticleSprite objects
class ParticleSystem : public BaseSystem
{
public:
  ParticleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  //! @brief Add ParticleSpriteBase<IParticle> via a list of ParticleSpriteOwner objects
  //! @param owners Contains a tag identifier and a unique_ptr to ParticleSpriteBase<IParticle>.
  void add( std::vector<ParticleSpriteOwner> owners );

  //! @brief Calls IParticle::update() function within all added ParticleSpriteBase<T>
  //! @param dt
  void update( sf::Time dt );

  //! @brief Find a ParticleSpriteOwner by tag and return a pointer to ParticleSpriteBase<IParticle>, or nullptr if not found
  [[nodiscard]] Cmp::IParticleSprite *find( const std::string &tag );

  //! @brief event handlers for pausing system clocks
  void onPause() override {};
  //! @brief event handlers for resuming system clocks
  void onResume() override {};
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PARTICLESYSTEM_HPP_