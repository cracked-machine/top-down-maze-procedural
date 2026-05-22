#ifndef SRC_UTILS_NPC_HPP_
#define SRC_UTILS_NPC_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <source_location>
namespace Game::Cmp
{
class Position;
}

namespace Game::Utils::Npc
{

entt::entity get_world_pos_entt( entt::registry &reg, Cmp::Position npc_pos );
entt::entity get_world_pos_entt( entt::registry &reg, entt::entity npc_entt );

Sprites::SpriteMetaType get_sprite_type( entt::registry &reg, entt::entity npc_entt, std::source_location loc = std::source_location::current() );

} // namespace Game::Utils::Npc

#endif // SRC_UTILS_NPC_HPP_
