#ifndef __COMPONENTS_DIJKSTRADISTANCE_HPP__
#define __COMPONENTS_DIJKSTRADISTANCE_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

struct DijkstraDistance {
    unsigned int distance = std::numeric_limits<unsigned int>::max();
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_DIJKSTRADISTANCE_HPP__