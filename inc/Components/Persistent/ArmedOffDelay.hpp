#ifndef __CMP__ARMEDOFFDELAY_HPP__
#define __CMP__ARMEDOFFDELAY_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being disarmed...violently
class ArmedOffDelay : public BasePersistent<float>
{

public:
  ArmedOffDelay( float delay )
      : BasePersistent<float>( delay )
  {
  }

  virtual std::string class_name() const override { return "ArmedOffDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP__ARMEDOFFDELAY_HPP__