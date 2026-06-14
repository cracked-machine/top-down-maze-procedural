#ifndef SRC_CMPS_STATS_CONSUMEACTION_HPP_
#define SRC_CMPS_STATS_CONSUMEACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace Game::Cmp
{

class ConsumeAction : public BaseAction
{
public:
  ConsumeAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                 Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~ConsumeAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_CMPS_STATS_CONSUMEACTION_HPP_