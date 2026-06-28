#ifndef SRC_PATHFINDING_SMARTPOINTERS_HPP__
#define SRC_PATHFINDING_SMARTPOINTERS_HPP__

namespace Game::PathFinding
{
class SpatialHashGrid;

using SpatialHashGridSharedPtr = std::shared_ptr<PathFinding::SpatialHashGrid>;
using SpatialHashGridWeakPtr = std::weak_ptr<PathFinding::SpatialHashGrid>;
using SpatialHashGridUniquePtr = std::unique_ptr<PathFinding::SpatialHashGrid>;

} // namespace Game::PathFinding

#endif // SRC_PATHFINDING_SMARTPOINTERS_HPP__
