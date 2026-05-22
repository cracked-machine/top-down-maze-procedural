#ifndef SRC_CMPS_STATS_CARRYACTION_HPP_
#define SRC_CMPS_STATS_CARRYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace Game::Cmp
{

class CarryAction : public BaseAction
{
public:
  CarryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick, Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~CarryAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_HOLDINGACTION_HPP_