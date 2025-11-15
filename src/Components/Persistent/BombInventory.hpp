#ifndef __CMP_BOMBINVENTORY_HPP__
#define __CMP_BOMBINVENTORY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class BombInventory : public BasePersistent<uint8_t>
{
public:
  BombInventory( uint8_t value = 10, uint8_t min_value = 0, uint8_t max_value = 100 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BombInventory"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBINVENTORY_HPP__
