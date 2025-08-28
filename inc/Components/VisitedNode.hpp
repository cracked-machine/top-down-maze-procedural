#ifndef __COMPONENTS_VISITEDNODE_HPP__
#define __COMPONENTS_VISITEDNODE_HPP__

#include <entt/entity/fwd.hpp>

namespace ProceduralMaze::Cmp {

// Used to mark entities for pathfinding. 
// Add to an entity to mark it as visited: `reg.emplace_or_replace<VisitedNode>(entity);`.
// Views can then exclude this component: `auto remaining = reg.view<Cmp::Obstacle>(entt::exclude<Cmp::VisitedNode>);`.
// Remove all when pathfinding is completed: `reg.clear<VisitedNode>();`
struct VisitedNode {
    // Constructor with no arguments to ensure it can be default constructed
    VisitedNode(bool visited) : m_visited(visited) {}

    // A non-const member to allow proper assignment
    bool m_visited{true};
};

} // namespace ProceduralMaze::Cmp

#endif // __COMPONENTS_VISITEDNODE_HPP__