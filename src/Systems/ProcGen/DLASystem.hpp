#ifndef SRC_SYSTEMS_PROCGEN_DLASYSTEM_HPP__
#define SRC_SYSTEMS_PROCGEN_DLASYSTEM_HPP__

#include <Systems/BaseSystem.hpp>

namespace Game::Sys::ProcGen
{

//! @brief Diffusion-limited aggregation
class DLASystem : public BaseSystem
{
public:
  //! @brief Shape used to bias/constrain where DLA particles are spawned from.
  enum class SpawnShape { Circle, Ellipse };

  //! @brief Construct a new DLASystem object
  DLASystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
      : BaseSystem( reg, window, sprite_factory, sound_bank )
  {
  }

  //! @brief event handlers for pausing system clocks
  void on_pause() override {}
  //! @brief event handlers for resuming system clocks
  void on_resume() override {}

  //! @brief DLA here works by removing obstacles (carving space) rather than adding them
  //! @param scene_size
  //! @param particle_limit
  //! @param max_radius
  //! @param levelgen_spatialgrid
  void iterate( sf::FloatRect scene_size, const sf::Vector2f seed_pos, uint16_t particle_limit, PathFinding::SpatialHashGrid &levelgen_spatialgrid );
};

} // namespace Game::Sys::ProcGen

#endif // SRC_SYSTEMS_PROCGEN_DLASYSTEM_HPP__
