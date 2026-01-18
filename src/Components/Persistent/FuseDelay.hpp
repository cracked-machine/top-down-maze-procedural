#ifndef __CMP_FUSEDELAY_HPP__
#define __CMP_FUSEDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class FuseDelay : public BasePersistent<float>
{
public:
  FuseDelay( float value = 2.5f, float min_value = 1.0f, float max_value = 10.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "FuseDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persist
#endif // __CMP_FUSEDELAY_HPP__