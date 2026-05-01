#ifndef SRC_SYSTEMS_PARTICLESYSTEM_HPP_
#define SRC_SYSTEMS_PARTICLESYSTEM_HPP_

#include <Events/ParticleEvents.hpp>
#include <Particle/ParticleSpriteBase.hpp>
#include <Systems/BaseSystem.hpp>
#include <ZOrderValue.hpp>
#include <utility>

#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys
{

// 1. Emission Stage:   - IParticle::emit()
// 2. Simulation Stage  - IParticleSprite::simulate()
// 3. Rendering Stage   - via RenderGameSystem using Cmp::ZorderValue

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
  std::unique_ptr<Cmp::Particle::IParticleSprite> sprite;

  explicit ParticleSpriteOwner( std::unique_ptr<Cmp::Particle::IParticleSprite> sprite )
      : sprite( std::move( sprite ) )
  {
  }
};

//! @brief Core system for adding and updating ParticleSprite objects
class ParticleSystem : public BaseSystem
{
public:
  ParticleSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank );

  template <typename... PARTICLESPRITES>
  void add( const std::pair<PARTICLESPRITES, Cmp::ZOrderValue> &...sprites )
  {
    auto add_one = [this]<typename PS>( const std::pair<PS, Cmp::ZOrderValue> &ps_pair )
    {
      const auto &[ps, zorder] = ps_pair;
      add_to_registry( ParticleSpriteOwner( std::make_unique<PS>( ps ) ), zorder );
    };

    ( add_one( sprites ), ... );
  }

  //! @brief Calls IParticle::update() function within all added ParticleSpriteBase<T>
  //! @param dt
  void update( sf::Time dt );

  void check_collsion( const sf::FloatRect &target );

  //! @brief Find a ParticleSpriteOwner by tag and return a pointer to ParticleSpriteBase<IParticle>, or nullptr if not found
  [[nodiscard]] static Cmp::Particle::IParticleSprite *find( entt::registry &reg, const std::string &tag );

  //! @brief event handlers for pausing system clocks
  void on_pause() override {};
  //! @brief event handlers for resuming system clocks
  void on_resume() override {};

private:
  //! @brief Add a wrapped particle sprite and zorder component to the registry
  //! @param owner
  //! @param zorder
  //! @return std::vector<entt::entity>
  std::vector<entt::entity> add_to_registry( ParticleSpriteOwner owner, Cmp::ZOrderValue zorder );
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PARTICLESYSTEM_HPP_