#ifndef __CMP_LANDDECELERATION_HPP__
#define __CMP_LANDDECELERATION_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class LandDeceleration : public BasePersistent<float>
{
public:
  LandDeceleration()
      : BasePersistent<float>( 600.0f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_LANDDECELERATION_HPP__
