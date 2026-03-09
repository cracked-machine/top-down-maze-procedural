#ifndef SRC_SYSTEMS_RUINSYSTEM_HPP_
#define SRC_SYSTEMS_RUINSYSTEM_HPP_

#include <Components/Ruin/RuinFloorAccess.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SpatialHashGrid.hpp>
#include <Systems/BaseSystem.hpp>

#include <Sprites/SpriteMetaType.hpp>
#include <future>
namespace ProceduralMaze::Sprites
{
class MultiSprite;
class SpriteFactory;
} // namespace ProceduralMaze::Sprites

namespace ProceduralMaze::Cmp
{
class RuinStairsLowerMultiBlock;
class RuinStairsUpperMultiBlock;
class RuinStairsBalustradeMultiBlock;
} // namespace ProceduralMaze::Cmp

namespace ProceduralMaze::Sys
{

class RuinSystem : public BaseSystem
{
public:
  RuinSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank,
              entt::dispatcher &scenemanager_event_dispatcher );

  //! @brief init the weak pointer for the spatial grid
  //! @param pathfinding_navmesh
  void init( const PathFinding::SpatialHashGridSharedPtr &pathfinding_navmesh ) { m_pathfinding_navmesh = pathfinding_navmesh; }

  void check_entrance_collision();
  void spawn_floor_access( sf::Vector2f spawn_position, sf::Vector2f size, Cmp::RuinFloorAccess::Direction dir );

  template <typename COMPONENT>
  void add_stairs( sf::Vector2f spawn_position, const Sprites::MultiSprite &stairs_ms, float zorder = 0 );

  void creaking_rope_update();

  void check_floor_access_collision( Cmp::RuinFloorAccess::Direction direction );

  //! @brief Reset the floor access cooldown (call when entering a ruin scene)
  void reset_floor_access_cooldown()
  {
    m_floor_access_cooldown.restart();
    m_was_on_floor_access = true; // Assume we just landed on a floor access
  }

  void gen_lowerfloor_bookcases( sf::FloatRect scene_dimensions );
  void add_lowerfloor_cobwebs( int max_attempts, sf::FloatRect scene_dimensions );
  void check_movement_slowdowns();
  void reset_player_curse();
  void check_exit_collision();

  //! @brief Create a witch entt if none exist
  //! @param reg
  //! @param spawn_position
  void check_create_witch( entt::registry &reg, sf::FloatRect scene_dimensions );

  //! @brief One time, one shot function that asynchronously enables player curse and then sleeps for N seconds.
  //!        This allows for cinematic pause after curse activation.
  // First frame — future is invalid → launch async, future becomes valid
  // Frames 2–N (still sleeping) — future is valid → skip launch, wait_for returns future_status::deferred or timeout
  // Frame after 10s — future is valid → skip launch, wait_for returns future_status::ready → set player_curse.active = true
  //! @param scene_dimensions
  //! @return true
  //! @return false
  bool check_activate_player_curse( sf::Vector2f scene_dimensions );
  bool is_player_carrying_witches_jar();
  void update_shadow_hand_pos( sf::Vector2f scene_dimensions );
  void check_player_shadow_hand_collision();

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

private:
  //! @brief Dispatcher reference for scene management events
  entt::dispatcher &m_scenemanager_event_dispatcher;

  //! @brief Cooldown clock to prevent immediate floor access re-triggering
  sf::Clock m_floor_access_cooldown{};
  static constexpr float kFloorAccessCooldownSeconds = 0.5f;

  //! @brief Track if player was on floor access last frame (requires leaving before re-triggering)
  bool m_was_on_floor_access{ false };

  sf::Clock m_creaking_rope_swing_timer{};
  std::future<void> m_curse_activation_future;

  PathFinding::SpatialHashGridWeakPtr m_pathfinding_navmesh;
};

extern template void RuinSystem::add_stairs<Cmp::RuinStairsLowerMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );
extern template void RuinSystem::add_stairs<Cmp::RuinStairsUpperMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );
extern template void RuinSystem::add_stairs<Cmp::RuinStairsBalustradeMultiBlock>( sf::Vector2f, const Sprites::MultiSprite &, float );

} // namespace ProceduralMaze::Sys

#endif // SRC_SYSTEMS_RUINSYSTEM_HPP_