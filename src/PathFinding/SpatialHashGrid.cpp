#include <SpatialHashGrid.hpp>

#include <Components/Position.hpp>

namespace ProceduralMaze::PathFinding
{

void SpatialHashGrid::insert( entt::entity e, const Cmp::Position &pos )
{
  auto [cx, cy] = cell( pos );
  grid[encode( cx, cy )].push_back( e );
}

void SpatialHashGrid::remove( entt::entity e, const Cmp::Position &pos )
{
  auto [cx, cy] = cell( pos );
  auto it = grid.find( encode( cx, cy ) );
  if ( it == grid.end() ) return;
  auto &bucket = it->second;
  std::erase( bucket, e );
}

void SpatialHashGrid::update( entt::entity e, const Cmp::Position &old_pos, const Cmp::Position &new_pos )
{
  remove( e, old_pos );
  insert( e, new_pos );
}

std::vector<entt::entity> SpatialHashGrid::query( const Cmp::Position &pos, QueryOffset offset ) const
{
  std::vector<entt::entity> result;
  auto [cx, cy] = cell( pos );

  static constexpr std::array<std::pair<int, int>, 5> cardinal_offsets{ {
      { 0, 0 },  // self
      { 0, -1 }, // up
      { 0, +1 }, // down
      { -1, 0 }, // left
      { +1, 0 }, // right
  } };

  static constexpr std::array<std::pair<int, int>, 9> all_offsets{ {
      { 0, 0 },   // self
      { 0, -1 },  // up
      { 0, +1 },  // down
      { -1, 0 },  // left
      { +1, 0 },  // right
      { -1, -1 }, // top-left
      { +1, -1 }, // top-right
      { -1, +1 }, // bottom-left
      { +1, +1 }, // bottom-right
  } };

  auto query_offsets = [&]( auto &offsets )
  {
    for ( auto [dx, dy] : offsets )
    {
      auto it = grid.find( encode( cx + dx, cy + dy ) );
      if ( it == grid.end() ) continue;
      result.insert( result.end(), it->second.begin(), it->second.end() );
    }
  };

  offset == QueryOffset::CARD ? query_offsets( cardinal_offsets ) : query_offsets( all_offsets );

  return result;
}

std::pair<int, int> SpatialHashGrid::cell( const Cmp::Position &pos ) const
{
  return { static_cast<int>( std::floor( pos.position.x / m_cell_size ) ), static_cast<int>( std::floor( pos.position.y / m_cell_size ) ) };
}

long long SpatialHashGrid::encode( int x, int y ) const
{
  // pack x and y into a single long long for use as a map key
  return ( static_cast<long long>( x ) << 32 ) | static_cast<unsigned int>( y );
}

} // namespace ProceduralMaze::PathFinding