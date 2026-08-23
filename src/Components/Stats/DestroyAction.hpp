#ifndef SRC_COMPONENTS_STATS_DESTROYACTION_HPP__
#define SRC_COMPONENTS_STATS_DESTROYACTION_HPP__

#include <Components/Stats/BaseAction.hpp>

namespace Game::Cmp
{

//! @brief Player-stat modifier applied when an entity (e.g. an NPC) is destroyed/removed, applied to
//! the player before the destroyed entity's components are removed.
class DestroyAction : public BaseAction
{
public:
  //! @brief Construct a new Destroy Action object.
  //! @param health Change applied to the health stat.
  //! @param fear Change applied to the fear stat.
  //! @param despair Change applied to the despair stat.
  //! @param infamy Change applied to the infamy stat.
  //! @param toxicity Change applied to the toxicity stat.
  //! @param tick How often (seconds) the action re-applies, or 0 for a one-shot.
  //! @param disease Disease affliction applied alongside the stat changes, if any.
  DestroyAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Toxicity toxicity, Stats::Tick tick,
                 Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, toxicity, tick, disease )
  {
  }
  //! @brief Destroy the Destroy Action object.
  ~DestroyAction() {}

private:
};

} // namespace Game::Cmp

#endif // SRC_COMPONENTS_STATS_DESTROYACTION_HPP__
