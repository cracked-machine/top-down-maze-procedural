#ifndef _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__
#define _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

//! @brief A percentage of damage that should be done to an item that is hit by player with tools
struct DiggingDamagePerHit : public BasePersistent<uint8_t>
{
  DiggingDamagePerHit( uint8_t value = 25, uint8_t min_value = 1, uint8_t max_value = 100 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "DiggingDamagePerHit"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // _PERSISTENT_DIGGING_DAMAGE_PER_HIT_HPP__