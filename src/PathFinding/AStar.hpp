#ifndef SRC_PATHFINDING_ASTAR_HPP_
#define SRC_PATHFINDING_ASTAR_HPP_

#include <Position.hpp>
#include <SpatialHashGrid.hpp>
#include <entt/entity/entity.hpp>

/// Main class
namespace ProceduralMaze::PathFinding
{

class SpatialHashGrid;

struct PathNode
{

  Cmp::Position pos;
  float x() { return pos.position.x; }
  float y() { return pos.position.y; }

  //! @brief Cost from start
  double g;
  //! @brief Heuristic cost to goal
  double h;
  //! @brief Pointer to parent node
  PathNode *parent{ nullptr };

  PathNode( Cmp::Position pos, double g = 0, double h = 0, PathNode *p = nullptr )
      : pos( pos ),
        g( g ),
        h( h ),
        parent( p )
  {
  }

  double f() const { return g + h; }
  bool operator==( const PathNode &other ) const { return pos.position.x == other.pos.position.x && pos.position.y == other.pos.position.y; }
};

std::vector<PathNode> astar( entt::registry &reg, const PathFinding::SpatialHashGrid &grid, Cmp::Position start, Cmp::Position goal,
                             PathFinding::QueryCompass query_compass = PathFinding::QueryCompass::CARDINAL );

} // namespace ProceduralMaze::PathFinding

#endif // SRC_PATHFINDING_ASTAR_HPP_