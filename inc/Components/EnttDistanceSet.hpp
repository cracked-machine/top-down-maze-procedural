#ifndef __CMP_ENTT_DISTANCE_SET_HPP__
#define __CMP_ENTT_DISTANCE_SET_HPP__

#include <cstddef>
#include <entt/entity/fwd.hpp>
#include <unordered_set>

namespace ProceduralMaze::Cmp {

// contains the entities that are within a certain distance, i.e. one grid
class EnttDistanceSet {

public:
    void set(const entt::entity& entity) { m_set.insert(entity); }
    std::optional<entt::entity> get(const entt::entity& entity) const 
    { 
        if (m_set.find(entity) != m_set.end())
        {
            return entity;
        }
        return std::nullopt;
    }
    void erase(const entt::entity& entity) { m_set.erase(entity); }
    std::size_t size() const { return m_set.size(); }
    std::size_t empty() const { return m_set.empty(); }
    void clear() { m_set.clear(); }
    auto begin() const { return m_set.begin(); } 
    auto end() const { return m_set.end(); }

private:
    std::unordered_set<entt::entity> m_set;
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_ENTT_DISTANCE_SET_HPP__

// TODO
// 1. Add a map here of K:Entity,V:EnttDistancesPriorityQueues
// 2. Add pair when within range
// 3. Remove pair when outside range