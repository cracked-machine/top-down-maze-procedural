#ifndef SRC_CMPS_STATS_CONSUMEACTION_HPP_
#define SRC_CMPS_STATS_CONSUMEACTION_HPP_

#include <Stats/BaseAction.hpp>

namespace ProceduralMaze::Cmp
{

class ConsumeAction : public BaseAction
{
public:
  ConsumeAction( Stats::Health health, Stats::Fear fear, Stats::Despair despair, Stats::Infamy infamy, Stats::Tick tick, Stats::Disease disease = {} )
      : BaseAction( health, fear, despair, infamy, tick, disease )
  {
  }
  ~ConsumeAction() {}

private:
};

} // namespace ProceduralMaze::Cmp

#endif // SRC_CMPS_STATS_CONSUMEACTION_HPP_