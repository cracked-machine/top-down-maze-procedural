#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <SFML/System/Clock.hpp>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// player concept
class PlayableCharacter {
public:
    bool alive{true};
    constexpr static const float MOVE_DIST{1.f};
    bool has_active_bomb{false}; // if the player has placed a bomb and it is active
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__