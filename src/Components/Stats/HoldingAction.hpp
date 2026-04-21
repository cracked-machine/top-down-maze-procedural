#ifndef SRC_CMPS_STATS_HOLDINGACTION_HPP_
#define SRC_CMPS_STATS_HOLDINGACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class HoldingAction : public BaseAction
{
public:
  HoldingAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy )
      : BaseAction( health, fear, despair, infamy )
  {
  }
  ~HoldingAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_HOLDINGACTION_HPP_