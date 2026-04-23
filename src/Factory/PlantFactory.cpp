#include <Components/AbsoluteAlpha.hpp>
#include <Components/Armable.hpp>
#include <Components/Inventory/InventoryItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/PlantFactory.hpp>
#include <Player/PlayerNoPath.hpp>
#include <SpatialHashGrid.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Utils.hpp>
#include <Utils/Random.hpp>

namespace ProceduralMaze::Factory
{

entt::entity create_plant_obstacle( entt::registry &reg, Cmp::Position pos_cmp, const Sprites::MultiSprite &ms )
{
  auto plant_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( plant_entt, pos_cmp.position, pos_cmp.size );
  reg.emplace_or_replace<Cmp::PlantObstacle>( plant_entt );
  reg.emplace_or_replace<Cmp::InventoryItem>( plant_entt, ms.get_sprite_type() );
  reg.emplace_or_replace<Cmp::ReservedPosition>( plant_entt );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( plant_entt );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( plant_entt, false );
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( plant_entt, 255 );
  reg.emplace_or_replace<Cmp::SpriteAnimation>( plant_entt, 0, 0, true, ms.get_sprite_type(), 0 );
  reg.emplace_or_replace<Cmp::Armable>( plant_entt );

  Cmp::ZOrderValue zorder( 0 );
  if ( ms.get_zorder( 0 ) != 0 ) { zorder.setZOrder( ms.get_zorder( 0 ) ); }
  else { zorder.setZOrder( pos_cmp.position.y ); }
  reg.emplace_or_replace<Cmp::ZOrderValue>( plant_entt, zorder );
  return plant_entt;
}

} // namespace ProceduralMaze::Factory
