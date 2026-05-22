#include <FootStepTimer.hpp>
#include <Npc.hpp>
#include <Npc/Npc.hpp>
#include <Player/PlayerCharacter.hpp>
#include <Position.hpp>
#include <Sprites/SpriteMetaType.hpp>
#include <entt/entity/fwd.hpp>
#include <source_location>
#include <stdexcept>

namespace Game::Utils::Npc
{

entt::entity get_world_pos_entt( entt::registry &reg, Cmp::Position npc_pos )
{

  auto excl = entt::exclude<Cmp::PlayerCharacter, Cmp::NPC, Cmp::FootStepTimer>;
  for ( auto [world_entt, world_pos_cmp] : reg.view<Cmp::Position>( excl ).each() )
  {
    if ( npc_pos.findIntersection( world_pos_cmp ) ) return world_entt;
  }
  return entt::null;
}

entt::entity get_world_pos_entt( entt::registry &reg, entt::entity npc_entt )
{
  auto *npc_pos = reg.try_get<Cmp::Position>( npc_entt );
  if ( not npc_pos ) return entt::null;

  auto excl = entt::exclude<Cmp::PlayerCharacter, Cmp::NPC, Cmp::FootStepTimer>;
  for ( auto [world_entt, world_pos_cmp] : reg.view<Cmp::Position>( excl ).each() )
  {
    if ( npc_pos->findIntersection( world_pos_cmp ) ) return world_entt;
  }
  return entt::null;
}

Sprites::SpriteMetaType get_sprite_type( entt::registry &reg, entt::entity npc_entt, std::source_location loc )
{
  auto loc_data = std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " - ";
  auto *npc_cmp = reg.try_get<Cmp::NPC>( npc_entt );
  if ( not npc_cmp ) throw std::runtime_error( loc_data + "Could not get NPC component from " + std::to_string( static_cast<uint32_t>( npc_entt ) ) );
  return npc_cmp->sprite_type_list.front();
}

} // namespace Game::Utils::Npc