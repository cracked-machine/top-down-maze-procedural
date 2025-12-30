#ifndef SRC_FACTORY_MULTIBLOCKFACTORY_HPP
#define SRC_FACTORY_MULTIBLOCKFACTORY_HPP

#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/CryptEntrance.hpp>
#include <Components/CryptObjectiveMultiBlock.hpp>
#include <Components/CryptObjectiveSegment.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <Systems/BaseSystem.hpp>

#include <Utils/Utils.hpp>
#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

template <typename MULTIBLOCK>
void createMultiblock( entt::registry &registry, entt::entity entity, Cmp::Position pos, const Sprites::MultiSprite &ms, int ms_idx = 0 )
{

  auto large_obst_grid_size = ms.get_grid_size();
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, ms.get_sprite_type(), ms_idx );
  registry.emplace_or_replace<MULTIBLOCK>( entity, pos.position, large_obst_grid_size.componentWiseMul( Constants::kGridSquareSizePixels ) );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, pos.position.y );

  auto zorder_cmp = registry.get<Cmp::ZOrderValue>( entity );
  // clang-format off
  SPDLOG_INFO( "Placed {} at position ({}, {}). Grid size: {}x{} at z-order {}", 
    ms.get_sprite_type(),
    pos.position.x,
    pos.position.y, 
    large_obst_grid_size.width, 
    large_obst_grid_size.height,
    zorder_cmp.getZOrder()
  );
  // clang-format on
}

template <typename MULTIBLOCK, typename MBSEGMENT>
void createMultiblockSegments( entt::registry &registry, entt::entity multiblock_entity, Cmp::Position pos, const Sprites::MultiSprite &ms )
{
  MULTIBLOCK new_multiblock_bounds = registry.get<MULTIBLOCK>( multiblock_entity );
  SPDLOG_INFO( "createMultiblockSegments called with MULTIBLOCK type: {} for {},{}", typeid( MBSEGMENT ).name(), pos.position.x, pos.position.y );

  auto pos_view = registry.view<Cmp::Position>();

  int intersection_count = 0;
  for ( auto [entity, pos_cmp] : pos_view.each() )
  {

    if ( not pos_cmp.findIntersection( new_multiblock_bounds ) ) continue;

    intersection_count++;
    SPDLOG_DEBUG( "Entity {} intersects multiblock at ({}, {})", static_cast<int>( entity ), pos_cmp.position.x, pos_cmp.position.y );

    // Only skip multiblock entity for CryptObjectiveSegment (they share the same entity)
    // Other multiblock types need their entity to be processed as a segment
    if constexpr ( std::is_same_v<MBSEGMENT, Cmp::CryptObjectiveSegment> )
    {
      if ( entity == multiblock_entity ) continue;
    }
    else
    {
      // For other segments, skip the multiblock entity to avoid z-order conflicts
    }

    // Calculate relative pixel positions within the large obstacle grid
    float rel_x = pos_cmp.position.x - pos.position.x;
    float rel_y = pos_cmp.position.y - pos.position.y;

    // Convert to relative grid coordinates
    int rel_grid_x = static_cast<int>( rel_x / Constants::kGridSquareSizePixels.x );
    int rel_grid_y = static_cast<int>( rel_y / Constants::kGridSquareSizePixels.y );

    std::size_t calculated_grid_index = rel_grid_y * ms.get_grid_size().width + rel_grid_x;
    SPDLOG_DEBUG( "  - Creating segment at ({}, {}) with sprite_index {}", pos_cmp.position.x, pos_cmp.position.y, calculated_grid_index );

    bool new_solid_mask = true;
    auto solid_masks = ms.get_solid_mask();
    if ( !solid_masks.empty() && solid_masks.size() > calculated_grid_index ) { new_solid_mask = solid_masks.at( calculated_grid_index ); }

    if ( new_solid_mask )
    {
      registry.emplace_or_replace<MBSEGMENT>( entity, new_solid_mask );
      registry.emplace_or_replace<Cmp::NoPathFinding>( entity );
    }
    registry.emplace_or_replace<Cmp::Armable>( entity );
    SPDLOG_INFO( "Modifying entity {}, sprite type {}, zorder to {}", static_cast<int>( entity ), ms.get_sprite_type(),
                 pos_cmp.position.y + ms.getSpriteSizePixels().y );

    // NOTE that this is a bit shit: hardcoded door placement for crypts.
    // If we add new MultiBlock sprites with 9+ segments they might suddenly sprout CryptDoors
    if ( calculated_grid_index == 10 )
    {
      registry.emplace_or_replace<Cmp::CryptEntrance>( entity );
      SPDLOG_INFO( "Adding Cmp::CryptEntrance at ({}, {}) with sprite_index {}", pos_cmp.position.x, pos_cmp.position.y, calculated_grid_index );
    }

    registry.emplace_or_replace<Cmp::ReservedPosition>( entity );

    SPDLOG_INFO( "Processed {} intersecting entities for multiblock {}", intersection_count, typeid( MULTIBLOCK ).name() );
  }
}

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_MULTIBLOCKFACTORY_HPP