#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// player concept
class PlayableCharacter {
public:
    // the source of all truthiness
    bool alive{true};
    // player health, zero means game over, man!
    uint8_t health{100};
    // unit movement
    constexpr static const float MOVE_DIST{1.f};
    // prevent bomb spamming
    sf::Clock m_bombdeploycooldowntimer;
    // prevent bomb spamming
    sf::Time m_bombdeploydelay{sf::seconds(0.5)};
    // prevent placing more than one bomb at a time
    bool has_active_bomb{false};
    // 0 is no bombs, -1 is infinite bombs
    int bomb_inventory{10}; 
    // blast radius for bomb explosions
    int blast_radius{1};  

    static sf::Vector2f PLAYER_START_POS;
};



} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__