#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class PlayerDetectionScale : public BasePersistent<float>
{
public:
  PlayerDetectionScale( float scale = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( scale, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDetectionScale"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PC_DETECTION_SCALE_HPP__