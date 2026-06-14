#ifndef SRC_CMPS_STATS_EXHUMEACTION_HPP_
#define SRC_CMPS_STATS_EXHUMEACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace Game::Cmp
{

class SpawnAction : public BaseAction
{
public:
  SpawnAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick, Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~SpawnAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_EXHUMEACTION_HPP_