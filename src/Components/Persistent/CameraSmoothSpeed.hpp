#ifndef __CMP_PERSIST_CAMERASMOOTHSPEED_HPP__
#define __CMP_PERSIST_CAMERASMOOTHSPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

//! @brief Controls the smoothing effect on the camera pan. Lower: smoother, Higher: snappier
struct CameraSmoothSpeed : BasePersistent<float>
{
  CameraSmoothSpeed( float value = 0, float min_value = 0, float max_value = 0 )
      : BasePersistent( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "CameraSmoothSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif