#ifndef SRC_PATHFINDING_SPATIALHASHGRID_HPP__
#define SRC_PATHFINDING_SPATIALHASHGRID_HPP__

#include <Utils/Constants.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>

namespace Game::Cmp
{
class Position;
}

namespace Game::PathFinding
{

//! @brief Select Cardinal or All offsets for query function
enum class QueryCompass {
  //! @brief Cardinal-direction offsets only (self, up, down, left, right).
  CARDINAL,
  //! @brief Ordinal-direction (diagonal) offsets only.
  ORDINAL,
  //! @brief Both cardinal and ordinal offsets (self plus all 8 neighbours).
  BOTH
};

//! @brief Store buckets of entities using their pixel positions as a key
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

  //! @brief Remove entire bucket using `pos` as a lookup
  //! @param pos
  void remove_all( const Cmp::Position &pos );

  //! @brief Remove `e` from its old position and re-insert at new position
  //! @param e
  //! @param old_pos
  //! @param new_pos
  void update( entt::entity e, const Cmp::Position &old_pos, const Cmp::Position &new_pos );

  //! @brief Get the entities in the bucket for `pos`, without including neighbouring buckets.
  //! @param pos
  //! @return The entities in that single bucket, or an empty vector if the bucket doesn't exist.
  std::vector<entt::entity> at( const Cmp::Position &pos ) const;

  //! @brief Using `pos` as a lookup, flattens neighbouring buckets (9 max) into single vector
  //! @param pos
  //! @return std::vector<entt::entity>
  std::vector<entt::entity> neighbours( const Cmp::Position &pos, QueryCompass offset = QueryCompass::BOTH ) const;

  //! @brief Get every entity whose bucket overlaps the axis-aligned rectangle `bounds` (world/pixel
  //! coordinates), flattening every matching bucket into one vector. Unlike neighbours(), which is
  //! anchored to a single position and covers at most a 3x3 cell block, this covers however many
  //! cells `bounds` actually spans - suited to a camera/view-bounds query rather than a
  //! single-entity proximity query.
  //! @param bounds World-space rectangle to query.
  //! @return Entities in every bucket the rectangle's cell-range overlaps, in no particular order.
  std::vector<entt::entity> query_rect( const sf::FloatRect &bounds ) const;

  //! @brief Get the number of occupied buckets in the grid.
  //! @return The bucket count.
  size_t size() { return m_grid.size(); }

  //! @brief Remove all buckets from the grid.
  void clear() { m_grid.clear(); }

private:
  //! @brief dimensions of a single cell in the game area grid
  static constexpr float m_cell_size{ Constants::kGridSizePxF.x };

  //! @brief spatial encoding of coords --> multiple entt bucket
  std::unordered_map<long long, std::vector<entt::entity>> m_grid;

  //! @brief Convert pixel coords into cell coords
  //! @param pos Anything position-like: Cmp::Position derives from sf::FloatRect, so it binds here too.
  //! @return std::pair<int, int>
  std::pair<int, int> cell( const sf::FloatRect &pos ) const;

  //! @brief Creates a bijective encoding of two x/y inputs into one output
  //! @param x
  //! @param y
  //! @return long long Packed x and y
  long long encode( int x, int y ) const;
};

} // namespace Game::PathFinding

#endif // SRC_PATHFINDING_SPATIALHASHGRID_HPP__
