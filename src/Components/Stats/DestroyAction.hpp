#ifndef SRC_CMPS_STATS_DESTROYACTION_HPP_
#define SRC_CMPS_STATS_DESTROYACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class DestroyAction : public BaseAction
{
public:
  DestroyAction( Health health, Fear fear, Despair despair, Infamy infamy )
      : BaseAction( health, fear, despair, infamy )
  {
  }
  ~DestroyAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_DESTROYACTION_HPP_