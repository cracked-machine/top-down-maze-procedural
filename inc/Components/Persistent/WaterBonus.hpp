#ifndef __CMP_WATERBONUS_HPP__
#define __CMP_WATERBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterBonus : public BasePersistent<float>
{
public:
  WaterBonus( float value = 100.0f )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERBONUS_HPP__
