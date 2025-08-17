#ifndef __COMPONENTS_PLAYABLECHARACTER_HPP__
#define __COMPONENTS_PLAYABLECHARACTER_HPP__

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp {

// player concept
class PlayableCharacter {
public:
    bool alive{true};
    constexpr static const float MOVE_DELTA{1.f};
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_PLAYABLECHARACTER_HPP__