#ifndef __CMP_WATERDECELERATION_HPP__
#define __CMP_WATERDECELERATION_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WaterDeceleration : public BasePersistent<float>
{
public:
  WaterDeceleration() : BasePersistent<float>( 90.0f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERDECELERATION_HPP__