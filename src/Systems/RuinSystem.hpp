#ifndef SRC_SYSTEMS_RUINSYSTEM_HPP_
#define SRC_SYSTEMS_RUINSYSTEM_HPP_

#include <Components/Ruin/RuinFloorAccess.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

class RuinSystem : public BaseSystem
{
public:
  RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief Update the player system.
  void update();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  // void spawn_objective( sf::Vector2f spawn_position, Sprites::SpriteMetaType type );
  void spawn_floor_access( sf::Vector2f spawn_position, sf::Vector2f size, Cmp::RuinFloorAccess::Direction dir );
  void spawn_staircase( sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms );

  void check_floor_access_collision( Cmp::RuinFloorAccess::Direction direction );

  //! @brief Reset the floor access cooldown (call when entering a ruin scene)
  void reset_floor_access_cooldown()
  {
    m_floor_access_cooldown.restart();
    m_was_on_floor_access = true; // Assume we just landed on a floor access
  }

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Cooldown clock to prevent immediate floor access re-triggering
  sf::Clock m_floor_access_cooldown{};
  static constexpr float kFloorAccessCooldownSeconds = 0.5f;

  //! @brief Track if player was on floor access last frame (requires leaving before re-triggering)
  bool m_was_on_floor_access{ false };
};

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_RUINSYSTEM_HPP_