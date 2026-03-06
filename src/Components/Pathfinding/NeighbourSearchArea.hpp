#ifndef SRC_CMPS_PATHFINDING_NEIGHBOURSEARCHAREA_HPP_
#define SRC_CMPS_PATHFINDING_NEIGHBOURSEARCHAREA_HPP_

#include <RectBounds.hpp>

namespace ProceduralMaze::Cmp
{

struct NeighbourSearchArea : public Cmp::RectBounds
{
  NeighbourSearchArea( Cmp::Position pos, float scale_factor )
      : RectBounds( pos, scale_factor )
  {
  }
  NeighbourSearchArea( sf::Vector2f pos, sf::Vector2f size, float scale_factor )
      : RectBounds( pos, size, scale_factor )
  {
  }
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_PATHFINDING_NEIGHBOURSEARCHAREA_HPP_