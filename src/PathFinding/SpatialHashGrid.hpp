#ifndef SRC_PATHFINDING_SPATIALHASHGRID_HPP_
#define SRC_PATHFINDING_SPATIALHASHGRID_HPP_

#include <Constants.hpp>

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>

namespace ProceduralMaze::Cmp
{
class Position;
}

namespace ProceduralMaze::PathFinding
{

//! @brief Select Cardinal or All offsets for query function
enum class QueryOffset {
  CARD, // cardinal only
  ALL   // include diagonals
};

class SpatialHashGrid
{
public:
  //! @brief Construct a new Spatial Hash Grid object
  SpatialHashGrid() = default;

  //! @brief insert entt `e` into the bucket for `pos`
  //! @param e
  //! @param pos
  void insert( entt::entity e, const Cmp::Position &pos );

  //! @brief Remove entt `e` from a bucket using `pos` as a lookup
  //! @param e
  //! @param pos
  void remove( entt::entity e, const Cmp::Position &pos );

  //! @brief Remove `e` from its old position and re-insert at new position
  //! @param e
  //! @param old_pos
  //! @param new_pos
  void update( entt::entity e, const Cmp::Position &old_pos, const Cmp::Position &new_pos );

  //! @brief Using `pos` as a lookup, flattens neighbouring buckets (9 max) into single vector
  //! @param pos
  //! @return std::vector<entt::entity>
  std::vector<entt::entity> query( const Cmp::Position &pos, QueryOffset offset = QueryOffset::ALL ) const;

private:
  //! @brief dimensions of a single cell in the game area grid
  static constexpr float m_cell_size{ Constants::kGridSizePxF.x };

  //! @brief spatial encoding of coords --> multiple entt bucket
  std::unordered_map<long long, std::vector<entt::entity>> grid;

  //! @brief Convert pixel coords into cell coords
  //! @param pos
  //! @return std::pair<int, int>
  std::pair<int, int> cell( const Cmp::Position &pos ) const;

  //! @brief Creates a bijective encoding of two x/y inputs into one output
  //! @param x
  //! @param y
  //! @return long long Packed x and y
  long long encode( int x, int y ) const;
};

} // namespace ProceduralMaze::PathFinding

#endif // SRC_PATHFINDING_SPATIALHASHGRID_HPP_