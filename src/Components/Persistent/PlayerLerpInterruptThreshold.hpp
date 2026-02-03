#ifndef __CMP_PERSIST_PLAYERLERPINTERRUPTTHRESHOLD_HPP__
#define __CMP_PERSIST_PLAYERLERPINTERRUPTTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

//! @brief Controls how far into a lerp the player can still change direction
//! 0.0 = no interruption allowed (must complete lerp)
//! 0.25 = can interrupt in first 25% of lerp
//! 0.5 = can interrupt in first 50% of lerp
//! 1.0 = can always interrupt (very responsive, but may feel jittery)
struct PlayerLerpInterruptThreshold : BasePersistent<float>
{
  PlayerLerpInterruptThreshold( float value = 0.f, float min = 0.f, float max = 0.f )
      : BasePersistent( value, min, max )
  {
  }
  virtual std::string class_name() const override { return "PlayerLerpInterruptThreshold"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PERSIST_PLAYERLERPINTERRUPTTHRESHOLD_HPP__