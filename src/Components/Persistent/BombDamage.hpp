#ifndef __CMP_BOMBDAMAGE_HPP__
#define __CMP_BOMBDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class BombDamage : public BasePersistent<uint8_t>
{
public:
  BombDamage( uint8_t value = 10, uint8_t min_value = 1, uint8_t max_value = 50 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "BombDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBDAMAGE_HPP__