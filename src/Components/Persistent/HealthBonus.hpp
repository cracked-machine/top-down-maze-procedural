#ifndef __CMP_HEALTHBONUS_HPP__
#define __CMP_HEALTHBONUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class HealthBonus : public BasePersistent<uint8_t>
{
public:
  HealthBonus( uint8_t value = 25, uint8_t min_value = 1, uint8_t max_value = 50 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "HealthBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_HEALTHBONUS_HPP__
