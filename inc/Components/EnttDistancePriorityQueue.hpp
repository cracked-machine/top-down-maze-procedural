#ifndef __CMP_ENTT_DISTANCE_PRIORITY_QUEUE_HPP__
#define __CMP_ENTT_DISTANCE_PRIORITY_QUEUE_HPP__

#include <entt/entity/fwd.hpp>
#include <queue>
#include <vector>
#include <utility> // for std::pair
#include <functional> // for std::greater

namespace ProceduralMaze::Cmp {

using EnttDistances = std::pair<int, entt::entity>;
using EnttDistancesPriorityQueue = std::priority_queue<EnttDistances, std::vector<EnttDistances>, std::greater<EnttDistances>>;

class EnttDistancePriorityQueue {

public:
    void push(const EnttDistances& distances) {
        m_queue.push(distances);
    }

    void pop() {
        m_queue.pop();
    }

    const EnttDistances& top() const {
        return m_queue.top();
    }

    bool empty() const {
        return m_queue.empty();
    }

    void clear() {
        m_queue = EnttDistancesPriorityQueue();
    }

    size_t size() const { return m_queue.size(); }

private:
    EnttDistancesPriorityQueue m_queue;
};

} // namespace ProceduralMaze::Cmp

#endif // __CMP_ENTT_DISTANCE_PRIORITY_QUEUE_HPP__

// TODO
// 1. Add a map here of K:Entity,V:EnttDistancesPriorityQueues
// 2. Add pair when within range
// 3. Remove pair when outside range