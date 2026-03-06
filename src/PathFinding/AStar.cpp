#include <Components/Position.hpp>
#include <Constants.hpp>
#include <PathFinding/AStar.hpp>
#include <Player/PlayerCharacter.hpp>
#include <SpatialHashGrid.hpp>
#include <Utils/Maths.hpp>
#include <Utils/Npc.hpp>

namespace ProceduralMaze::PathFinding
{

std::vector<PathNode> astar( entt::registry &reg, const PathFinding::SpatialHashGrid &spatial_grid, Cmp::Position start, Cmp::Position goal,
                             PathFinding::QueryCompass offset )
{

  auto contains = [&]( const std::list<PathNode> &vec, const PathNode &node )
  { return std::any_of( vec.begin(), vec.end(), [&]( const PathNode &n ) { return n == node; } ); };

  std::vector<PathNode> openList;
  std::list<PathNode> closedList;

  PathNode startNode( start, 0, Utils::Maths::getManhattanDistance( start.position, goal.position ) );

  openList.push_back( startNode );

  PathNode *endNode = nullptr;

  while ( !openList.empty() )
  {
    // Find PathNode with smallest f
    auto currentIt = std::min_element( openList.begin(), openList.end(), []( const PathNode &a, const PathNode &b ) { return a.f() < b.f(); } );
    PathNode current = *currentIt;
    openList.erase( currentIt );
    closedList.push_back( current );

    if ( current.x() == goal.x() && current.y() == goal.y() )
    {
      endNode = &closedList.back();
      break;
    }

    const std::vector<entt::entity> neighbours_list = spatial_grid.query( current.pos, offset );

    for ( auto neighbour_entt : neighbours_list )
    {
      auto *neighbour_pos = reg.try_get<Cmp::Position>( neighbour_entt );
      if ( not neighbour_pos ) continue;

      // if ( !inBounds( nx, ny ) ) continue;

      auto heuristic = Utils::Maths::getManhattanDistance( neighbour_pos->position, goal.position );
      PathNode new_neighbor( *neighbour_pos, current.g + 1, heuristic, &closedList.back() );

      if ( contains( closedList, new_neighbor ) ) continue;

      auto it = std::find_if( openList.begin(), openList.end(),
                              // existence check
                              [&]( const PathNode &n ) { return n == new_neighbor; } );

      // either add the new neighbour or add duplicate one if this is nearer to the goal
      if ( it == openList.end() || new_neighbor.g < it->g )
      {
        if ( it != openList.end() ) openList.erase( it );
        openList.push_back( new_neighbor );
      }
    }
  }

  // Reconstruct path
  std::vector<PathNode> path;
  PathNode *p = endNode;

  while ( p != nullptr )
  {
    path.push_back( *p );
    p = p->parent;
  }

  std::reverse( path.begin(), path.end() );
  return path;
}

} // namespace ProceduralMaze::PathFinding