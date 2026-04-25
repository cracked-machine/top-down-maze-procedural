#ifndef SRC_CMPS_STATS_BURYACTION_HPP_
#define SRC_CMPS_STATS_BURYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class BuryAction : public BaseAction
{
public:
  BuryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick, Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~BuryAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_BURYACTION_HPP_