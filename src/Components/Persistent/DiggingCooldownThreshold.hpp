#ifndef _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__
#define _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

struct DiggingCooldownThreshold : public BasePersistent<float>
{
  DiggingCooldownThreshold( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "DiggingCooldownThreshold"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__