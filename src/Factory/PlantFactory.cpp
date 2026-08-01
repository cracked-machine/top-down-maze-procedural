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
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <entt/entity/registry.hpp>
#include <vector>

namespace Game::Factory::Plant
{

void remove_plant_mb( entt::registry &reg, entt::entity plant_entt, const PathFinding::SpatialHashGridSharedPtr &npc_navmesh,
                      const PathFinding::SpatialHashGridSharedPtr &player_navmesh )
{
  auto *plant_mb_cmp = reg.try_get<Cmp::PlantMultiBlock>( plant_entt );
  auto *plant_uuid_cmp = reg.try_get<Cmp::UUID>( plant_entt );
  if ( not plant_mb_cmp or not plant_uuid_cmp ) return;

  // Collect all segment entities for this plant, then destroy them.
  // Segment entities are purpose-built (not world tiles), so destroy rather than strip components.
  std::vector<entt::entity> to_destroy;
  std::vector<Cmp::Position> segment_positions;
  for ( auto [other_uuid_entt, plant_segment_cmp, seg_pos_cmp, other_uuid_cmp] : reg.view<Cmp::PlantSegment, Cmp::Position, Cmp::UUID>().each() )
  {
    if ( *plant_uuid_cmp == other_uuid_cmp )
    {
      SPDLOG_INFO( "Found segment entt {}", static_cast<uint32_t>( other_uuid_entt ) );
      to_destroy.push_back( other_uuid_entt );
      segment_positions.push_back( seg_pos_cmp );
    }
  }
  for ( std::size_t i = 0; i < to_destroy.size(); ++i )
  {
    // segments are indexed in the player navmesh as blockers - drop them before destroying,
    // otherwise is_valid_move queries stale entity handles
    if ( player_navmesh ) player_navmesh->remove( to_destroy[i], segment_positions[i] );
    if ( reg.valid( to_destroy[i] ) ) reg.destroy( to_destroy[i] );
  }

  // Un-reserve the world tiles this plant claimed in create_multiblock_segments, otherwise
  // replanting here silently skips segment creation. Match tiles against the destroyed
  // segments' positions so reservations held by neighbouring structures are untouched.
  for ( auto [tile_entt, tile_pos_cmp, tile_reserved_cmp] : reg.view<Cmp::Position, Cmp::ReservedPosition>().each() )
  {
    for ( const auto &seg_pos : segment_positions )
    {
      if ( tile_pos_cmp.findIntersection( seg_pos ) )
      {
        reg.remove<Cmp::ReservedPosition>( tile_entt );
        // the tile is walkable again for NPCs, unless it blocks in its own right (e.g. an obstacle)
        if ( npc_navmesh && not reg.any_of<Cmp::NpcNoPathFinding>( tile_entt ) ) { npc_navmesh->insert( tile_entt, tile_pos_cmp ); }
        break;
      }
    }
  }

  // now destroy the redundant plant multiblock entity
  if ( reg.valid( plant_entt ) ) reg.destroy( plant_entt );
}

} // namespace Game::Factory::Plant
