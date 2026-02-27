#ifndef SRC_CMPS_PERSISTENT_PLAYERNUDGESPEEDMULTIPLIER_HPP_
#define SRC_CMPS_PERSISTENT_PLAYERNUDGESPEEDMULTIPLIER_HPP_

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class PlayerNudgeSpeedMultiplier : public BasePersistent<float>
{
public:
  PlayerNudgeSpeedMultiplier( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerNudgeSpeedMultiplier"; }
  const std::string get_detail() const override { return detail; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_CMPS_PERSISTENT_PLAYERNUDGESPEEDMULTIPLIER_HPP_