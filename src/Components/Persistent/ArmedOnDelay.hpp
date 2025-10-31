#ifndef __CMP_ARMEDONDELAY_HPP__
#define __CMP_ARMEDONDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being armed
class ArmedOnDelay : public BasePersistent<float>
{
public:
  ArmedOnDelay( float value )
      : BasePersistent<float>( value )
  {
  }

  virtual std::string class_name() const override { return "ArmedOnDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_ARMEDONDELAY_HPP__