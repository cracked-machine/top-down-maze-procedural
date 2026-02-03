#ifndef __CMP_FUSEDELAY_HPP__
#define __CMP_FUSEDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class FuseDelay : public BasePersistent<float>
{
public:
  FuseDelay( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "FuseDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persist
#endif // __CMP_FUSEDELAY_HPP__