#ifndef SRC_FACTORY_MULTIBLOCKFACTORY_HPP
#define SRC_FACTORY_MULTIBLOCKFACTORY_HPP

#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/CryptEntrance.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/Position.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Systems/BaseSystem.hpp>

#include <entt/entt.hpp>

namespace ProceduralMaze::Factory
{

template <typename MULTIBLOCK>
void createMultiblock( entt::registry &registry, entt::entity entity, Cmp::Position pos,
                       const Sprites::MultiSprite &ms )
{

  auto large_obst_grid_size = ms.get_grid_size();
  registry.emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, ms.get_sprite_type(), 0 );
  registry.emplace_or_replace<MULTIBLOCK>( entity, pos.position,
                                           large_obst_grid_size.componentWiseMul( Constants::kGridSquareSizePixels ) );
  registry.emplace_or_replace<Cmp::ZOrderValue>( entity, pos.position.y + ms.getSpriteSizePixels().y );

  // clang-format off
  SPDLOG_INFO( "Placed {} at position ({}, {}). Grid size: {}x{}", 
    ms.get_sprite_type(),
    pos.position.x,
    pos.position.y, 
    large_obst_grid_size.width, 
    large_obst_grid_size.height 
  );
  // clang-format on
}

template <typename MULTIBLOCK, typename MBSEGMENT>
void createMultiblockSegments( entt::registry &registry, entt::entity multiblock_entity, Cmp::Position pos,
                               const Sprites::MultiSprite &ms )
{
  MULTIBLOCK new_multiblock_bounds = registry.get<MULTIBLOCK>( multiblock_entity );

  auto pos_view = registry.view<Cmp::Position>();
  for ( auto [entity, pos_cmp] : pos_view.each() )
  {
    if ( pos_cmp.findIntersection( new_multiblock_bounds ) )
    {
      // Calculate relative pixel positions within the large obstacle grid
      float rel_x = pos_cmp.position.x - pos.position.x;
      float rel_y = pos_cmp.position.y - pos.position.y;

      // Convert to relative grid coordinates
      int rel_grid_x = static_cast<int>( rel_x / Constants::kGridSquareSizePixels.x );
      int rel_grid_y = static_cast<int>( rel_y / Constants::kGridSquareSizePixels.y );

      // Calculate linear array index using relative grid distance from the origin grid position
      // [0,0]. We can then use the index to look up the sprite and solid mask in the large obstacle
      // sprite object (method: row-major order: index = y * width + x) Example for a 4x2 grid:
      //         [0][1][2][3]
      //         [4][5][6][7]
      // Top-left position: grid_y=0, grid_x=0 → sprite_index = 0 * 4 + 0 = 0
      // Top-right position: grid_y=0, grid_x=3 → sprite_index = 0 * 4 + 3 = 3
      // Bottom-left position: grid_y=1, grid_x=0 → sprite_index = 1 * 4 + 0 = 4
      // Bottom-right position: grid_y=1, grid_x=3 → sprite_index = 1 * 4 + 3 = 7
      std::size_t calculated_grid_index = rel_grid_y * ms.get_grid_size().width + rel_grid_x;
      SPDLOG_DEBUG( "  - Creating segment at ({}, {}) with sprite_index {}", pos_cmp.position.x, pos_cmp.position.y,
                    calculated_grid_index );

      bool new_solid_mask = true;
      auto solid_masks = ms.get_solid_mask();
      if ( !solid_masks.empty() && solid_masks.size() > calculated_grid_index )
      {
        new_solid_mask = solid_masks.at( calculated_grid_index );
      }

      if ( new_solid_mask )
      {
        registry.emplace_or_replace<MBSEGMENT>( entity, new_solid_mask );
        registry.emplace_or_replace<Cmp::NoPathFinding>( entity );
      }
      registry.emplace_or_replace<Cmp::Armable>( entity );
      registry.emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + ms.getSpriteSizePixels().y );

      // NOTE that this is a bit shit: hardcoded door placement for crypts.
      // If we add new MultiBlock sprites with 9+ segments they might suddenly sprout CryptDoors
      if ( calculated_grid_index == 10 )
      {
        registry.emplace_or_replace<Cmp::CryptEntrance>( entity );
        SPDLOG_INFO( "Adding Cmp::CryptEntrance at ({}, {}) with sprite_index {}", pos_cmp.position.x,
                     pos_cmp.position.y, calculated_grid_index );
      }

      registry.emplace_or_replace<Cmp::ReservedPosition>( entity );
    }
  }
}

} // namespace ProceduralMaze::Factory

#endif // SRC_FACTORY_MULTIBLOCKFACTORY_HPP