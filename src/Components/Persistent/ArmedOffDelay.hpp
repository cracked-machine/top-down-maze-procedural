#ifndef __CMP__ARMEDOFFDELAY_HPP__
#define __CMP__ARMEDOFFDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being disarmed...violently
class ArmedOffDelay : public BasePersistent<float>
{
public:
  ArmedOffDelay( float delay = 0.075f, float min_value = 0.001f, float max_value = 0.5f )
      : BasePersistent<float>( delay, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "ArmedOffDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP__ARMEDOFFDELAY_HPP__