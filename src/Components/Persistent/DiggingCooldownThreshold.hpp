#ifndef _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__
#define _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

struct DiggingCooldownThreshold : public BasePersistent<float>
{
  DiggingCooldownThreshold( float value = 0.4f, float min_value = 0.05f, float max_value = 1.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "DiggingCooldownThreshold"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__