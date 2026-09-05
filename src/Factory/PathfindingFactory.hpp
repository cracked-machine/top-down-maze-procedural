#ifndef SRC_FACTORY_PATHFINDINGFACTORY_HPP__
#define SRC_FACTORY_PATHFINDINGFACTORY_HPP__

#include <PathFinding/SmartPointers.hpp>

#include <functional>

namespace Game::Factory::Pathfinding
{

//! @brief Decides whether a NpcNoPathFinding entity blocks a given navmesh variant.
//! Return false to let this navmesh's users pass through that blocker (e.g. ghosts
//! through plant segments, or future NPC types blocked only by specific plant types).
using NpcBlockerFilter = std::function<bool( entt::entity )>;

//! @brief Build the navmesh used by regular NPC pathfinding: every world position, minus any tile tagged
//! Cmp::Npc::NoPathFinding.
//! @param reg
//! @return The new navmesh.
PathFinding::SpatialHashGridSharedPtr create_npc_navmesh( entt::registry &reg );

//! @brief Build the navmesh used by ghost pathfinding: every world position, minus any tile tagged
//! Cmp::Npc::NoPathFinding.
//! @param reg
//! @return The new navmesh.
PathFinding::SpatialHashGridSharedPtr create_ghost_navmesh( entt::registry &reg );

//! @brief Build the navmesh used by player movement collision: only tiles tagged Cmp::Player::NoPath.
//! @param reg
//! @return The new navmesh.
PathFinding::SpatialHashGridSharedPtr create_player_navmesh( entt::registry &reg );

//! @brief Build a navmesh containing every world position, with no blockers applied.
//! @param reg
//! @return The new navmesh.
PathFinding::SpatialHashGridSharedPtr create_open_navmesh( entt::registry &reg );

//! @brief Clears and repopulates an existing grid in place with every entity eligible for
//! RenderGameSystem::add_visible_entity_to_z_order_queue()'s Cmp::Position fast path - see
//! create_render_position_grid()'s docs for exactly which entities and why.
//! @note Reuses the same PathFinding::SpatialHashGrid object rather than replacing it, specifically so
//! a periodic re-populate (e.g. GraveyardScene's kRenderPositionGridRebuildInterval) doesn't invalidate
//! any weak_ptr another system (e.g. InventorySystem) is already holding to it.
//! @param reg
//! @param render_position_grid The grid to clear and repopulate.
void populate_render_position_grid( entt::registry &reg, PathFinding::SpatialHashGrid &render_position_grid );

//! @brief Build the spatial index RenderGameSystem::add_visible_entity_to_z_order_queue() queries for
//! its Cmp::Position pass: every entity with Cmp::Position + Cmp::AnimData + Cmp::ZOrderValue that is
//! static once created (obstacles, plants, loot, multiblock roots/segments, etc.), excluding both the
//! movers (player, NPCs, arrows, moveable obstacles + their caps - see
//! RenderGameSystem::add_mover_to_z_order_queue(), which handles these separately via a plain scan) and
//! the 6 multiblock root types that already get their own dedicated pass.
//! @note Call this once, after level generation has finished creating all static content (mirroring
//! create_npc_navmesh() et al.) - it only captures what already exists in `reg` at the time it runs.
//! Anything created afterwards at runtime (loot drops, bombs, footstep decals, dug/replanted
//! obstacles/plants, ...) needs its own insert()/remove() calls at the relevant factory/system code
//! wherever it's created or destroyed, matching this same exclude list - or gets picked up eventually by
//! a periodic populate_render_position_grid() call.
//! @param reg
//! @return The new grid.
PathFinding::SpatialHashGridSharedPtr create_render_position_grid( entt::registry &reg );

} // namespace Game::Factory::Pathfinding

#endif // SRC_FACTORY_PATHFINDINGFACTORY_HPP__
