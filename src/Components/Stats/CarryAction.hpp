#ifndef SRC_CMPS_STATS_CARRYACTION_HPP_
#define SRC_CMPS_STATS_CARRYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class CarryAction : public BaseAction
{
public:
  CarryAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Disease disease = Stats::Disease::NONE )
      : BaseAction( health, fear, despair, infamy, disease )
  {
  }
  ~CarryAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_HOLDINGACTION_HPP_