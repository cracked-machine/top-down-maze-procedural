#ifndef SRC_COMPONENTS_NPC_SHOCKWAVETIMER_HPP__
#define SRC_COMPONENTS_NPC_SHOCKWAVETIMER_HPP__

#include <SFML/System/Clock.hpp>

namespace Game::Cmp::Npc
{

//! @brief Per-NPC cooldown clock tracking time since the NPC's last emitted shockwave (see
//! Factory::Npc::create_shockwave), compared against Cmp::Persist::NpcShockwaveFreq.
struct ShockwaveTimer : sf::Clock
{
};

} // namespace Game::Cmp::Npc

#endif // SRC_COMPONENTS_NPC_SHOCKWAVETIMER_HPP__
