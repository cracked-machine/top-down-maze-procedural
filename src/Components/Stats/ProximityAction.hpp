#ifndef SRC_CMPS_STATS_PROXIMITYACTION_HPP_
#define SRC_CMPS_STATS_PROXIMITYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace Game::Cmp
{

class ProximityAction : public BaseAction
{
public:
  ProximityAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick,
                   Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~ProximityAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_PROXIMITYACTION_HPP_