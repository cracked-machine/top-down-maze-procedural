#ifndef SRC_CMPS_STATS_SACRIFICEACTION_HPP_
#define SRC_CMPS_STATS_SACRIFICEACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{
//! @brief Construct a new Sacrifice Action object
//! @param health
//! @param fear
//! @param despair
//! @param infamy
class SacrificeAction : public BaseAction
{
public:
  SacrificeAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick,
                   Stats::Disease disease = Stats::Disease::NONE )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~SacrificeAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_SACRIFICEACTION_HPP_