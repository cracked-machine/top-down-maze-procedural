#ifndef __SYS_PATHFINDSYSTEM_HPP__
#define __SYS_PATHFINDSYSTEM_HPP__

#include <Components/EnttDistanceMap.hpp>
#include <Components/LerpPosition.hpp>
#include <Components/NPC.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Direction.hpp>
#include <NPCScanBounds.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/NpcLerpSpeed.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <Systems/BaseSystem.hpp>

#include <cstdlib>

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <array>
#include <cmath>

#include <queue>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys {

using PlayerDistanceQueue = std::priority_queue<std::pair<int, entt::entity>, std::vector<std::pair<int, entt::entity>>,
                                                std::greater<std::pair<int, entt::entity>>>;

class PathFindSystem : public BaseSystem
{
public:
  PathFindSystem( ProceduralMaze::SharedEnttRegistry reg );
  ~PathFindSystem() = default;

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