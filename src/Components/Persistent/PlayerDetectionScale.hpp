#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent
{

class PlayerDetectionScale : public BasePersistent<float>
{
public:
  PlayerDetectionScale( float scale = 8.0, float min_value = 1.0f, float max_value = 20.0f )
      : BasePersistent<float>( scale, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDetectionScale"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PC_DETECTION_SCALE_HPP__