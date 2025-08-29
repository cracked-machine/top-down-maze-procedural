#ifndef __COMPONENTS_NPC_HPP__
#define __COMPONENTS_NPC_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

class NPC {
public:
    NPC(bool npc) : m_is_npc(npc) {}
    [[maybe_unused]] bool m_is_npc{true};
    sf::Clock m_move_cooldown;
    const sf::Time MOVE_DELAY{sf::seconds(2.f)}; // NPC moves every 2 seconds
private:
    // This is redundant but ENTT requires components have at least one member
};
} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPC_HPP__

