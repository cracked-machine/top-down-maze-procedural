#ifndef _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__
#define _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent
{

struct DiggingDamagePerHit : public BasePersistent<uint8_t>
{
  DiggingDamagePerHit( uint8_t value = 32, uint8_t min_value = 64, uint8_t max_value = 128 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "DiggingDamagePerHit"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__