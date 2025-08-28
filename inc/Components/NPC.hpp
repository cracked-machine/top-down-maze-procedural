#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

class NPC {
public:
    NPC(bool npc) : m_is_npc(npc) {}
    [[maybe_unused]] bool m_is_npc{true};
private:
    // This is redundant but ENTT requires components have at least one member
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__

