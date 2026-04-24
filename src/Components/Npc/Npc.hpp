#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <Components/Stats/BaseAction.hpp>
#include <Sprites/SpriteMetaType.hpp>

#include <typeindex>

namespace ProceduralMaze::Cmp
{

class NPC
{
public:
  NPC() = default;
  NPC( Sprites::SpriteMetaType type )
      : type( std::vector<Sprites::SpriteMetaType>{ std::move( type ) } )
  {
  }
  NPC( std::vector<Sprites::SpriteMetaType> type_list )
      : type( std::move( type_list ) )
  {
  }
  // bool active = true;

  //! @brief The associated sprite
  std::vector<Sprites::SpriteMetaType> type;

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, BaseAction> action_fx_map;
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__
