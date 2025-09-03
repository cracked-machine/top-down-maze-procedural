#ifndef __CMP_ENTT_DISTANCE_MAP_HPP__
#define __CMP_ENTT_DISTANCE_MAP_HPP__

#include <cstddef>
#include <entt/entity/fwd.hpp>
#include <unordered_map>

namespace ProceduralMaze::Cmp {

// contains the entities that are within a certain distance, i.e. one grid
class EnttDistanceMap {

public:
  void set(const entt::entity &entity, int value) { m_map[entity] = value; }
  std::optional<int> get(const entt::entity &entity) const {
    if (m_map.find(entity) != m_map.end()) {
      return m_map.at(entity);
    }
    return std::nullopt;
  }
  std::size_t size() const { return m_map.size(); }
  std::size_t empty() const { return m_map.empty(); }
  void clear() { m_map.clear(); }
  auto begin() const { return m_map.begin(); }
  auto end() const { return m_map.end(); }

private:
  std::unordered_map<entt::entity, int> m_map;
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_ENTT_DISTANCE_MAP_HPP__

// TODO
// 1. Add a map here of K:Entity,V:EnttDistancesPriorityQueues
// 2. Add pair when within range
// 3. Remove pair when outside range