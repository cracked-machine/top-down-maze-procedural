#ifndef __CMP_LANDACCELERATION_HPP__
#define __CMP_LANDACCELERATION_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class LandAcceleration : public BasePersistent<float>
{
public:
  LandAcceleration()
      : BasePersistent<float>( 500.0f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_LANDACCELERATION_HPP__