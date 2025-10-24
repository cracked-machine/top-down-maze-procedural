#ifndef _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__
#define _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

struct DiggingDamagePerHit : public BasePersistent<float>
{
  DiggingDamagePerHit()
      : BasePersistent<float>( 1.0f )
  {
  }
  virtual std::string class_name() const override { return "DiggingDamagePerHit"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__