#ifndef SRC_CMPS_STATS_DESTROYACTION_HPP_
#define SRC_CMPS_STATS_DESTROYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class DestroyAction : public BaseAction
{
public:
  DestroyAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Disease disease = Stats::Disease::NONE )
      : BaseAction( health, fear, despair, infamy, disease )
  {
  }
  ~DestroyAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_DESTROYACTION_HPP_