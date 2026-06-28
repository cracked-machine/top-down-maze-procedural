#ifndef SRC_COMPONENTS_STATS_SPAWNACTION_HPP__
#define SRC_COMPONENTS_STATS_SPAWNACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

class SpawnAction : public BaseAction
{
public:
  SpawnAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
               Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~SpawnAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_SPAWNACTION_HPP__
