#ifndef __CMP_BOMBINVENTORY_HPP__
#define __CMP_BOMBINVENTORY_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombInventory : public BasePersistent<int>
{
public:
  BombInventory( int value = 10 )
      : BasePersistent<int>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBINVENTORY_HPP__
