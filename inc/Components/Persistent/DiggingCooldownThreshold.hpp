#ifndef _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__
#define _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

struct DiggingCooldownThreshold : public BasePersistent<float>
{
  DiggingCooldownThreshold()
      : BasePersistent<float>( 1.0f )
  {
  }
  virtual std::string class_name() const override { return "DiggingCooldownThreshold"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_DIGGING_COOLDOWN_THRESHOLD_HPP__