#include <Components/PlayerKeysCount.hpp>
#include <Components/PlayerRelicCount.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Window.hpp>

#include <Components/CorruptionCell.hpp>
#include <Components/Destructable.hpp>
#include <Components/Exit.hpp>
#include <Components/FootStepTimer.hpp>
#include <Components/GraveSprite.hpp>
#include <Components/HazardFieldCell.hpp>
#include <Components/LargeObstacle.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Obstacle.hpp>
#include <Components/Persistent/BombBonus.hpp>
#include <Components/Persistent/CorruptionDamage.hpp>
#include <Components/Persistent/HealthBonus.hpp>
#include <Components/Persistent/NpcDamage.hpp>
#include <Components/Persistent/NpcPushBack.hpp>
#include <Components/Persistent/PcDamageDelay.hpp>
#include <Components/Persistent/ShrineCost.hpp>
#include <Components/Persistent/WaterBonus.hpp>
#include <Components/PlayerCandlesCount.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SelectedPosition.hpp>
#include <Components/ShrineSprite.hpp>
#include <Components/SinkholeCell.hpp>
#include <Components/SpawnAreaSprite.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/WeaponLevel.hpp>
#include <Events/NpcCreationEvent.hpp>
#include <Events/PlayerActionEvent.hpp>
#include <Events/UnlockDoorEvent.hpp>
#include <Sprites/MultiSprite.hpp>
#include <Systems/CollisionSystem.hpp>
#include <Systems/Render/RenderSystem.hpp>

namespace ProceduralMaze::Sys {

CollisionSystem::CollisionSystem( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window,
                                  Sprites::SpriteFactory &sprite_factory )
    : BaseSystem( reg, window, sprite_factory )
{

  SPDLOG_DEBUG( "CollisionSystem initialized" );
}

void CollisionSystem::suspend()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
  for ( auto [_pc_entt, player] : player_collision_view.each() )
  {
    if ( player.m_bombdeploycooldowntimer.isRunning() ) player.m_bombdeploycooldowntimer.stop();
  }
}
void CollisionSystem::resume()
{
  auto player_collision_view = m_reg->view<Cmp::PlayableCharacter>();
  for ( auto [_pc_entt, player] : player_collision_view.each() )
  {
    if ( not player.m_bombdeploycooldowntimer.isRunning() ) player.m_bombdeploycooldowntimer.start();
  }
}

} // namespace ProceduralMaze::Sys