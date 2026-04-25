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
  NPC( Sprites::SpriteMetaType sprite_type )
      : sprite_type_list( std::vector<Sprites::SpriteMetaType>{ std::move( sprite_type ) } )
  {
  }
  NPC( std::vector<Sprites::SpriteMetaType> sprite_type_list )
      : sprite_type_list( std::move( sprite_type_list ) )
  {
  }
  // bool active = true;

  //! @brief The associated sprite
  std::vector<Sprites::SpriteMetaType> sprite_type_list;

  struct ActionTimePair
  {
    BaseAction action;
    sf::Time time;
  };

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, ActionTimePair> actions;
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__
