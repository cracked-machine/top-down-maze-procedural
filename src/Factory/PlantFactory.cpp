#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
#include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

namespace Game::Factory
{

entt::entity create_plant_obstacle( entt::registry &reg, Cmp::Position pos_cmp, const Sprites::SpriteSheet &ms )
{
  // make sure we mark all position entities as reserved to prevent procgen reusing this spot.
  for ( auto [existing_pos_entt, existing_pos_cmp] : reg.view<Cmp::Position>().each() )
  {
    if ( not pos_cmp.findIntersection( existing_pos_cmp ) ) continue;
    reg.emplace_or_replace<Cmp::ReservedPosition>( existing_pos_entt );
  }

  auto plant_entt = reg.create();
  reg.emplace_or_replace<Cmp::Position>( plant_entt, pos_cmp.position, pos_cmp.size );
  reg.emplace_or_replace<Cmp::PlantObstacle>( plant_entt );
  reg.emplace_or_replace<Cmp::WorldItem>( plant_entt, ms.get_sprite_type().substr( std::string_view{ "sprite." }.size() ), ms.get_sprite_type() );
  reg.emplace_or_replace<Cmp::ReservedPosition>( plant_entt );
  reg.emplace_or_replace<Cmp::NpcNoPathFinding>( plant_entt );
  reg.emplace_or_replace<Cmp::PlayerNoPath>( plant_entt, false );
  reg.emplace_or_replace<Cmp::AbsoluteAlpha>( plant_entt, 255 );
  // clang-format off
  reg.emplace_or_replace<Cmp::AnimData>( plant_entt, Cmp::AnimData::Config{ 
        .sprite_type = ms.get_sprite_type(), 
        .enabled = true
  });
  // clang-format on
  reg.emplace_or_replace<Cmp::Armable>( plant_entt );

  Cmp::ZOrderValue zorder( 0 );
  if ( ms.get_zorder( 0 ) != 0 ) { zorder.setZOrder( ms.get_zorder( 0 ) ); }
  else { zorder.setZOrder( pos_cmp.position.y ); }
  reg.emplace_or_replace<Cmp::ZOrderValue>( plant_entt, zorder );
  return plant_entt;
}

} // namespace Game::Factory
