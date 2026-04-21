#ifndef SRC_CMPS_STATS_BURYACTION_HPP_
#define SRC_CMPS_STATS_BURYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class BuryAction : public BaseAction
{
public:
  BuryAction( Health health, Fear fear, Despair despair, Infamy infamy )
      : BaseAction( health, fear, despair, infamy )
  {
  }
  ~BuryAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_BURYACTION_HPP_