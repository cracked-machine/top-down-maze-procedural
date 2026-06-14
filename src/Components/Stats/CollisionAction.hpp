#ifndef SRC_CMPS_STATS_COLLISIONACTION_HPP_
#define SRC_CMPS_STATS_COLLISIONACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace Game::Cmp
{
//! @brief Construct a new Sacrifice Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
class CollisionAction : public BaseAction
{
public:
  CollisionAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                   Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~CollisionAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_COLLISIONACTION_HPP_