#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <SFML/Graphics/Rect.hpp>
#include <spdlog/spdlog.h>

#include <Components/Direction.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>

#include <Components/Obstacle.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/NpcLerpSpeed.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Systems/BaseSystem.hpp>

#include <array>
#include <cmath>
#include <cstdlib>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <queue>

namespace ProceduralMaze::Sys {

using PlayerDistanceQueue = std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>,
                                                std::greater<std::pair<int, entt::entity>>>;

class PathFindSystem : public BaseSystem
{
public:
  PathFindSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                  Audio::SoundBank &sound_bank );
  ~PathFindSystem() = default;

  //! @brief event handlers for pausing system clocks
  void onPause() override {}
  //! @brief event handlers for resuming system clocks
  void onResume() override {}

  void findPath( entt::entity player_entity );

  // Update distances from player to obstacles for traversable obstacles
  // Only obstacles within the PCDetectionBounds are considered.
  // Use F3 to visualize the distance values (pixels from player center)
  void update_player_distances();

private:
  void scanForPlayers( entt::entity npc_entity, entt::entity player_entity );

  // Define possible movement directions (up, right, down, left)
  const std::array<sf::Vector2f, 4> m_directions = {
      sf::Vector2f( 0.f, -BaseSystem::kGridSquareSizePixels.y ), // Up
      sf::Vector2f( BaseSystem::kGridSquareSizePixels.x, 0.f ),  // Right
      sf::Vector2f( 0.f, BaseSystem::kGridSquareSizePixels.y ),  // Down
      sf::Vector2f( -BaseSystem::kGridSquareSizePixels.x, 0.f )  // Left
  };
};

} // namespace ProceduralMaze::Sys

#endif // __SYS_PATHFINDSYSTEM_HPP__