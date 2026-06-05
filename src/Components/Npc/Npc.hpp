#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <Components/Stats/BaseAction.hpp>
#include <Sprites/SpriteMetaType.hpp>

#include <typeindex>

namespace Game::Cmp
{

class NPC
{
public:
  NPC() = default;
  NPC( Sprites::SpriteMetaType sprite_type, float lerp_speed, float frame_rate )
      : sprite_type_list( std::vector<Sprites::SpriteMetaType>{ std::move( sprite_type ) } ),
        m_lerp_speed( lerp_speed ),
        m_frame_rate( frame_rate )
  {
  }
  NPC( std::vector<Sprites::SpriteMetaType> sprite_type_list, float lerp_speed, float frame_rate )
      : sprite_type_list( std::move( sprite_type_list ) ),
        m_lerp_speed( lerp_speed ),
        m_frame_rate( frame_rate )
  {
  }
  // bool active = true;

  //! @brief The associated sprite. Supports multiple sprites for animation. See res/json/npc.json.
  std::vector<Sprites::SpriteMetaType> sprite_type_list;

  struct ActionTimePair
  {
    BaseAction action;
    sf::Time time;
  };

  //! @brief The action and its effects that can be applied to the player
  std::unordered_map<std::type_index, ActionTimePair> actions;

  float m_lerp_speed{0};
  float m_frame_rate{0};
};
} // namespace Game::Cmp

#endif // __COMPONENTS_NPC_HPP__
