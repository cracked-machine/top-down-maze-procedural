#ifndef SRC_COMPONENTS_STATS_SACRIFICEACTION_HPP__
#define SRC_COMPONENTS_STATS_SACRIFICEACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{
//! @brief Construct a new Sacrifice Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
class SacrificeAction : public BaseAction
{
public:
  SacrificeAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                   Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  ~SacrificeAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_SACRIFICEACTION_HPP__
