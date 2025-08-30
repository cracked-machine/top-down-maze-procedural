#ifndef __COMPONENTS_NPCDISTANCE_HPP__
#define __COMPONENTS_NPCDISTANCE_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {
// Component to store the distance from NPCs to obstacles
struct NPCDistance {
    unsigned int distance = std::numeric_limits<unsigned int>::max();
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_NPCDISTANCE_HPP__