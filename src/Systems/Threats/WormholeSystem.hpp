#ifndef __SYS_WORMHOLE_SYSTEM_HPP__
#define __SYS_WORMHOLE_SYSTEM_HPP__

#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

#include <Systems/BaseSystem.hpp>

namespace ProceduralMaze::Sys
{

// 1. Wormhole spawns at random location
// 2. Player collision with wormhole
// 3. Player is teleported to random location
// 4. Wormhole despawns
// 5. Repeat
class WormholeSystem : public BaseSystem
{
public:
  WormholeSystem( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                  Audio::SoundBank &sound_bank );

  //! @brief event handlers for pausing system clocks
  void onPause() override;
  //! @brief event handlers for resuming system clocks
  void onResume() override;

  enum class SpawnPhase
  {
    InitialSpawn,
    Respawn
  };
  // 1. pick a random position component in the maze
  // 2. get the entity at that position
  // 3. remove the entities obstacle component
  // 4. add the wormhole component to the entity
  void spawn_wormhole( SpawnPhase phase );

  std::pair<entt::entity, Cmp::Position> find_spawn_location( unsigned long seed );

  // 1. iterate wormhole and player view,
  // 2. check for collision,
  // 3. if collision, pick a random location
  // 4. remove the obstacle from the location
  // 5. teleport player to the location
  // 6. call despawn_wormhole()
  // 7. call spawn_wormhole()
  void check_player_wormhole_collision();

  // remove the wormhole entity
  void despawn_wormhole();
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_WORMHOLE_SYSTEM_HPP__