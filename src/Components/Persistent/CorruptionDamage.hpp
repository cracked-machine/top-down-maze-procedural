#ifndef __CMP_CORRUPTIONDAMAGE_HPP__
#define __CMP_CORRUPTIONDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class CorruptionDamage : public BasePersistent<uint8_t>
{
public:
  CorruptionDamage( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "CorruptionDamage"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_CORRUPTIONDAMAGE_HPP__