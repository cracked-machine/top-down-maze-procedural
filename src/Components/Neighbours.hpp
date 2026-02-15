#ifndef __COMPONENTS_NEIGHBOURS_HPP__
#define __COMPONENTS_NEIGHBOURS_HPP__

#include <map>
#include <string>

namespace ProceduralMaze::Cmp
{

//! @brief Holds neighbour info for an entity. Used by CellAutomata
class Neighbours
{
public:
  enum class Dir { UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT };

  void set( Dir dir, entt::entity entity_u32 );
  std::optional<entt::entity> get( Dir dir );

  std::string to_string( Dir dir ) const;
  void clear();

  void remove( Dir dir );

  int count() const;

  auto begin();
  auto end();

private:
  // map of neighbour entity IDs and their relative direction
  std::map<Dir, entt::entity> m_entities;
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NEIGHBOURS_HPP__