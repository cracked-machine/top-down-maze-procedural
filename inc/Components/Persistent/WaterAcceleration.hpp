#ifndef __CMP_WATERACCELERATION_HPP__
#define __CMP_WATERACCELERATION_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterAcceleration : public BasePersistent<float>
{
public:
  WaterAcceleration()
      : BasePersistent<float>( 250.0f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERACCELERATION_HPP__