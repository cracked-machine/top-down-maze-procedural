#ifndef SRC_COMPONENTS_STATS_DESTROYACTION_HPP__
#define SRC_COMPONENTS_STATS_DESTROYACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

class DestroyAction : public BaseAction
{
public:
  DestroyAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                 Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~DestroyAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_DESTROYACTION_HPP__
