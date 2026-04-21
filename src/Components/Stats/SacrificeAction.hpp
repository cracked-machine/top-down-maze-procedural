#ifndef SRC_CMPS_STATS_SACRIFICEACTION_HPP_
#define SRC_CMPS_STATS_SACRIFICEACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class SacrificeAction : public BaseAction
{
public:
  SacrificeAction( Health health, Fear fear, Despair despair, Infamy infamy )
      : BaseAction( health, fear, despair, infamy )
  {
  }
  ~SacrificeAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_SACRIFICEACTION_HPP_