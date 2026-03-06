#ifndef SRC_UTILS_NPC_HPP_
#define SRC_UTILS_NPC_HPP_

namespace ProceduralMaze::Cmp
{
class Position;
}

namespace ProceduralMaze::Utils::Npc
{

entt::entity get_world_pos_entt( entt::registry &reg, Cmp::Position npc_pos );
entt::entity get_world_pos_entt( entt::registry &reg, entt::entity npc_entt );

} // namespace ProceduralMaze::Utils::Npc

#endif // SRC_UTILS_NPC_HPP_
