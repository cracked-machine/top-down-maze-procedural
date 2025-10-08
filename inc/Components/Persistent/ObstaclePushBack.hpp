#ifndef __CMP_OBSTACLEPUSHBACK_HPP__
#define __CMP_OBSTACLEPUSHBACK_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class ObstaclePushBack : public BasePersistent<float>
{
public:
  ObstaclePushBack( float value = 1.1f )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "ObstaclePushBack"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_OBSTACLEPUSHBACK_HPP__