#include <Components/AbsoluteAlpha.hpp>
#include <Components/AnimData.hpp>
#include <Components/Armable.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Inventory/WorldItem.hpp>
#include <Components/Npc/NpcNoPathFinding.hpp>
// #include <Components/PlantObstacle.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Player/PlayerNoPath.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/UUID.hpp>
#include <Components/ZOrderValue.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/SpriteFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/registry.hpp>
#include <vector>

namespace Game::Factory
{

void remove_plant_mb( entt::registry &reg, entt::entity plant_entt )
{
  auto *plant_mb_cmp = reg.try_get<Cmp::PlantMultiBlock>( plant_entt );
  auto *plant_uuid_cmp = reg.try_get<Cmp::UUID>( plant_entt );
  if ( not plant_mb_cmp or not plant_uuid_cmp ) return;

  // find the UUID for the other multiblock/segments and remove them. We want to retain the entity/position.
  for ( auto [other_uuid_entt, plant_segment_cmp, other_uuid_cmp] : reg.view<Cmp::PlantSegment, Cmp::UUID>().each() )
  {
    if ( not Utils::is_visible_in_view( Sys::RenderSystem::get_world_view(), *plant_mb_cmp ) ) continue;
    if ( *plant_uuid_cmp == other_uuid_cmp )
    {
      SPDLOG_INFO( "Found segment entt {}", static_cast<uint32_t>( other_uuid_entt ) );
      reg.remove<Cmp::PlantMultiBlock>( other_uuid_entt );
      reg.remove<Cmp::PlantSegment>( other_uuid_entt );
      reg.remove<Cmp::UUID>( other_uuid_entt );
      reg.remove<Cmp::PlayerNoPath>( other_uuid_entt );
      reg.remove<Cmp::NpcNoPathFinding>( other_uuid_entt );
      reg.remove<Cmp::ReservedPosition>( other_uuid_entt );
    }
  }
  // now destroy the redundant plant multiblock entity
  if ( reg.valid( plant_entt ) ) reg.destroy( plant_entt );
}

} // namespace Game::Factory
