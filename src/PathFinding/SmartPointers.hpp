#ifndef SRC_PATHFINDING_SMARTPOINTERS_HPP__
#define SRC_PATHFINDING_SMARTPOINTERS_HPP__

namespace Game::PathFinding
{
class SpatialHashGrid;

//! @brief Shared-ownership pointer alias for SpatialHashGrid, used where the NPC navmesh is co-owned by
//! multiple systems.
using SpatialHashGridSharedPtr = std::shared_ptr<PathFinding::SpatialHashGrid>;

//! @brief Non-owning pointer alias for SpatialHashGrid, used where a system only needs to observe the NPC
//! navmesh without extending its lifetime.
using SpatialHashGridWeakPtr = std::weak_ptr<PathFinding::SpatialHashGrid>;

//! @brief Sole-ownership pointer alias for SpatialHashGrid.
using SpatialHashGridUniquePtr = std::unique_ptr<PathFinding::SpatialHashGrid>;

} // namespace Game::PathFinding

#endif // SRC_PATHFINDING_SMARTPOINTERS_HPP__
