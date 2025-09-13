#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <Components/Armed.hpp>
#include <Components/Direction.hpp>
#include <Components/Loot.hpp>
#include <Components/Movement.hpp>
#include <Components/NPC.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <NPCScanBounds.hpp>
#include <NpcSystem.hpp>
#include <PCDetectionBounds.hpp>
#include <Persistent/BombBonus.hpp>
#include <Persistent/FrictionCoefficient.hpp>
#include <Persistent/FrictionFalloff.hpp>
#include <Persistent/HealthBonus.hpp>
#include <Persistent/LandMaxSpeed.hpp>
#include <Persistent/NPCActivateScale.hpp>
#include <Persistent/NpcDamage.hpp>
#include <Persistent/NpcDamageDelay.hpp>
#include <Persistent/NpcPushBack.hpp>
#include <Persistent/ObstaclePushBack.hpp>
#include <Persistent/WaterBonus.hpp>
#include <Persistent/WaterMaxSpeed.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>

#include <cassert>
#include <cmath>
#include <memory>

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <spdlog/spdlog.h>

#define assertm( exp, msg ) assert( ( void( msg ), exp ) )

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem
{
public:
  CollisionSystem( std::shared_ptr<entt::basic_registry<entt::entity>> reg ) : BaseSystem( reg ) {}

  ~CollisionSystem() = default;

  // Initialise persistent components as ECS context variables
  void init_context();

  // Pause timers when game is paused
  void suspend();

  // Resume timers when game is unpaused
  void resume();

  // Check for player collision with bones obstacles to reanimate NPCs
  void check_bones_reanimation();

  // Check for player collision with NPCs
  void check_player_to_npc_collision();

  // Check for player collision with loot items i.e. player picks up loot
  void check_loot_collision();

  // Check for player collision with end zone to complete level
  void check_end_zone_collision();

  // Update distances from player to obstacles for traversable obstacles
  // Only obstacles within the PCDetectionBounds are considered.
  // Use F3 to visualize the distance values (pixels from player center)
  void update_obstacle_distances();

  // Check for player collision with obstacles. This is the main collision detection function
  void check_player_obstacle_collision();

private:
  sf::FloatRect m_end_zone{ { kDisplaySize.x * 1.f, 0 }, { 500.f, kDisplaySize.y * 1.f } };
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__