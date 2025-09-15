#ifndef __CMP_OBSTACLEPUSHBACK_HPP__
#define __CMP_OBSTACLEPUSHBACK_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class ObstaclePushBack : public BasePersistent<float>
{
public:
  ObstaclePushBack()
      : BasePersistent<float>( 1.1f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_OBSTACLEPUSHBACK_HPP__