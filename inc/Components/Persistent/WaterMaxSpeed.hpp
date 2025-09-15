#ifndef __CMP_WATERMAXSPEED_HPP__
#define __CMP_WATERMAXSPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterMaxSpeed : public BasePersistent<float>
{
public:
  WaterMaxSpeed()
      : BasePersistent<float>( 50.0f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERMAXSPEED_HPP__