#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// player concept
class PlayableCharacter {
public:
    bool alive{true};
    uint8_t health{100};
    constexpr static const float MOVE_DIST{1.f};
    sf::Clock m_bombdeploycooldowntimer;
    sf::Time m_bombdeploydelay{sf::seconds(0.5)};
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__