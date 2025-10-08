#ifndef __CMP_PC_DETECTION_SCALE_HPP__
#define __CMP_PC_DETECTION_SCALE_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class PlayerDetectionScale : public BasePersistent<float>
{
public:
  PlayerDetectionScale( float scale = 7.0f )
      : BasePersistent<float>( scale )
  {
  }
  virtual std::string class_name() const override { return "PlayerDetectionScale"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PC_DETECTION_SCALE_HPP__