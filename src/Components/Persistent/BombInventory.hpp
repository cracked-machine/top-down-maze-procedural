#ifndef __CMP_BOMBINVENTORY_HPP__
#define __CMP_BOMBINVENTORY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombInventory : public BasePersistent<int>
{
public:
  BombInventory( int value = 10, int min_value = -1, int max_value = 100 )
      : BasePersistent<int>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BombInventory"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBINVENTORY_HPP__
