#ifndef SRC_SYSTEMS_PARTICLESYSTEM_HPP_
#define SRC_SYSTEMS_PARTICLESYSTEM_HPP_

#include <Events/ParticleEvents.hpp>
#include <Particle/ParticleSpriteBase.hpp>
#include <Systems/BaseSystem.hpp>
#include <utility>

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
  void add( const PARTICLESPRITES &...sprites )
  {
    auto add_one = [this]<typename T>( const T &ps )
    {
      std::vector<Sys::ParticleSpriteOwner> owners;
      owners.emplace_back( std::make_unique<T>( ps ) );
      add_to_registry( std::move( owners ) );
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
  void onPause() override {};
  //! @brief event handlers for resuming system clocks
  void onResume() override {};

private:
  // //! @brief Prepare registry insertion by wrapping with ParticleSpriteOwner
  // //! @tparam PARTICLESPRITE
  // //! @param ps
  // template <typename PARTICLESPRITE>
  // void add( const PARTICLESPRITE &ps )
  // {
  //   std::vector<Sys::ParticleSpriteOwner> owners;
  //   owners.emplace_back( std::make_unique<PARTICLESPRITE>( ps ) );
  //   add_to_registry( std::move( owners ) );
  // }

  //! @brief Add ParticleSpriteBase<IParticle> via a list of ParticleSpriteOwner objects
  //! @param owners Contains a tag identifier and a unique_ptr to ParticleSpriteBase<IParticle>.
  std::vector<entt::entity> add_to_registry( std::vector<ParticleSpriteOwner> owners );
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_PARTICLESYSTEM_HPP_