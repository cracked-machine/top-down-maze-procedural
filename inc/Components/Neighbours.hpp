#ifndef __COMPONENTS_NEIGHBOURS_HPP__
#define __COMPONENTS_NEIGHBOURS_HPP__

#include <spdlog/spdlog.h>

#include <entt/entity/registry.hpp>
#include <map>
#include <string>

namespace ProceduralMaze::Cmp {

class Neighbours
{
public:
  enum class Dir
  {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
  };

  void set( Dir dir, entt::entity entity_u32 ) { m_entities[dir] = entity_u32; }

  std::optional<entt::entity> get( Dir dir )
  {
    if ( m_entities.find( dir ) != m_entities.end() ) { return m_entities[dir]; }
    else { return std::nullopt; }
  }

  std::string to_string( Dir dir ) const
  {
    std::string result;
    if ( dir == Dir::UP )
      result = "UP";
    else if ( dir == Dir::DOWN )
      result = "DOWN";
    else if ( dir == Dir::LEFT )
      result = "LEFT";
    else if ( dir == Dir::RIGHT )
      result = "RIGHT";
    else if ( dir == Dir::UP_LEFT )
      result = "UP_LEFT";
    else if ( dir == Dir::UP_RIGHT )
      result = "UP_RIGHT";
    else if ( dir == Dir::DOWN_LEFT )
      result = "DOWN_LEFT";
    else if ( dir == Dir::DOWN_RIGHT )
      result = "DOWN_RIGHT";
    else
      SPDLOG_ERROR( "Unknown direction: {}", static_cast<int>( dir ) );
    return result;
  }
  void clear() { m_entities.clear(); }

  void remove( Dir dir )
  {
    if ( m_entities.find( dir ) != m_entities.end() ) { m_entities.erase( dir ); }
    else { SPDLOG_WARN( "Attempted to remove non-existent entity in direction: {}", static_cast<int>( dir ) ); }
  }

  int count() const { return m_entities.size(); }

  auto begin() { return m_entities.begin(); }
  auto end() { return m_entities.end(); }

private:
  // map of neighbour entity IDs and their relative direction
  std::map<Dir, entt::entity> m_entities;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NEIGHBOURS_HPP__