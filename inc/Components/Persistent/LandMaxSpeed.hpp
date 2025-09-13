#ifndef __CMP_LANDMAXSPEED_HPP__
#define __CMP_LANDMAXSPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class LandMaxSpeed : public BasePersistent<float>
{
public:
  LandMaxSpeed() : BasePersistent<float>( 100.0f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_LANDMAXSPEED_HPP__