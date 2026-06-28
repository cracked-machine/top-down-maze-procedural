#ifndef SRC_COMPONENTS_STATS_CARRYACTION_HPP__
#define SRC_COMPONENTS_STATS_CARRYACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

class CarryAction : public BaseAction
{
public:
  CarryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
               Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~CarryAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_CARRYACTION_HPP__
