#ifndef __SYSTEMS_COLLISION_SYSTEM_HPP__
#define __SYSTEMS_COLLISION_SYSTEM_HPP__

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Window.hpp>

#include <spdlog/spdlog.h>

#include <Components/Armed.hpp>
#include <Components/CorruptionCell.hpp>
#include <Components/Direction.hpp>
#include <Components/Loot.hpp>
#include <Components/NPC.hpp>
#include <Components/NPCScanBounds.hpp>
#include <Components/Neighbours.hpp>
#include <Components/Obstacle.hpp>
#include <Components/PCDetectionBounds.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/PlayerDistance.hpp>
#include <Components/Position.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/System.hpp>
#include <Components/WaterLevel.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Systems/Threats/NpcSystem.hpp>

#include <cassert>
#include <cmath>

#define assertm( exp, msg ) assert( ( void( msg ), exp ) )

namespace ProceduralMaze::Sys {

class CollisionSystem : public BaseSystem
{
public:
  CollisionSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                   Audio::SoundBank &sound_bank );

  ~CollisionSystem() = default;

  //! @brief event handlers for pausing system clocks
  void onPause() override;
  //! @brief event handlers for resuming system clocks
  void onResume() override;

private:
};

} // namespace ProceduralMaze::Sys

#endif // __SYSTEMS_COLLISION_SYSTEM_HPP__