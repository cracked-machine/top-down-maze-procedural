#ifndef SRC_CMPS_STATS_PROJECTILEACTION_HPP_
#define SRC_CMPS_STATS_PROJECTILEACTION_HPP_

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{
//! @brief Construct a new Sacrifice Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
class ProjectileAction : public BaseAction
{
public:
  ProjectileAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                    Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~ProjectileAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_PROJECTILEACTION_HPP_