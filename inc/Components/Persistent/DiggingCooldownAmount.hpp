#ifndef _PERSISTENT_DIGGING_COOLDOWN_AMOUNT_HPP__
#define _PERSISTENT_DIGGING_COOLDOWN_AMOUNT_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

struct DiggingCooldownAmount : public BasePersistent<float>
{
  DiggingCooldownAmount()
      : BasePersistent<float>( 1.0f )
  {
  }
  virtual std::string class_name() const override { return "DiggingCooldownAmount"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_DIGGING_COOLDOWN_AMOUNT_HPP__