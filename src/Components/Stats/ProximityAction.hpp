#ifndef SRC_COMPONENTS_STATS_PROXIMITYACTION_HPP__
#define SRC_COMPONENTS_STATS_PROXIMITYACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

class ProximityAction : public BaseAction
{
public:
  ProximityAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                   Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~ProximityAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_PROXIMITYACTION_HPP__
