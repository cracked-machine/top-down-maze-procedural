#ifndef SRC_CMPS_STATS_BURYACTION_HPP_
#define SRC_CMPS_STATS_BURYACTION_HPP_

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

class BuryAction : public BaseAction
{
public:
  BuryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
              Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~BuryAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_BURYACTION_HPP_