#ifndef SRC_CMPS_STATS_COLLISIONACTION_HPP_
#define SRC_CMPS_STATS_COLLISIONACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{
//! @brief Construct a new Sacrifice Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
class CollisionAction : public BaseAction
{
public:
  CollisionAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy,
                   Stats::Disease disease = Stats::Disease::NONE, Stats::Tick tick = Stats::Tick::ONCE )
      : BaseAction( health, fear, despair, infamy, disease, tick )
  {
  }
  ~CollisionAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_COLLISIONACTION_HPP_