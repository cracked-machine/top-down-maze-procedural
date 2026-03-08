#ifndef SRC_PATHFINDING_SMARTPOITNER_HPP_
#define SRC_PATHFINDING_SMARTPOITNER_HPP_

namespace ProceduralMaze::PathFinding
{
class SpatialHashGrid;

using SpatialHashGridSharedPtr = std::shared_ptr<PathFinding::SpatialHashGrid>;
using SpatialHashGridWeakPtr = std::weak_ptr<PathFinding::SpatialHashGrid>;

} // namespace ProceduralMaze::PathFinding

#endif // SRC_PATHFINDING_SMARTPOITNER_HPP_