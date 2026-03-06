#include <FootStepTimer.hpp>
#include <Npc.hpp>
#include <Npc/Npc.hpp>
#include <Player/PlayerCharacter.hpp>
#include <Position.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Utils::Npc
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

} // namespace ProceduralMaze::Utils::Npc