#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class PlayerDetectionScale : public BasePersistent<float>
{
public:
  PlayerDetectionScale( float scale = 5.0, float min_value = 1.0f, float max_value = 20.0f )
      : BasePersistent<float>( scale, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDetectionScale"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PC_DETECTION_SCALE_HPP__