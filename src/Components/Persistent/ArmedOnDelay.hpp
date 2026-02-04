#ifndef __CMP_ARMEDONDELAY_HPP__
#define __CMP_ARMEDONDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

// Delay between each concentric obstacle in bomb pattern being armed
class ArmedOnDelay : public BasePersistent<float>
{
public:
  ArmedOnDelay( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "ArmedOnDelay"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_ARMEDONDELAY_HPP__