#ifndef SRC_COMPONENTS_STATS_BURYACTION_HPP__
#define SRC_COMPONENTS_STATS_BURYACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

//! @brief Player-stat modifier applied when an item or cadaver is buried.
class BuryAction : public BaseAction
{
public:
  //! @brief Construct a new Bury Action object.
  //! @param health Change applied to the health stat.
  //! @param fear Change applied to the fear stat.
  //! @param despair Change applied to the despair stat.
  //! @param infamy Change applied to the infamy stat.
  //! @param toxicity Change applied to the toxicity stat.
  //! @param tick How often (seconds) the action re-applies, or 0 for a one-shot.
  //! @param disease Disease affliction applied alongside the stat changes, if any.
  BuryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
              Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  //! @brief Destroy the Bury Action object.
  ~BuryAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_BURYACTION_HPP__
