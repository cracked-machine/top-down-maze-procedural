#ifndef __CMP_FUSEDELAY_HPP__
#define __CMP_FUSEDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class FuseDelay : public BasePersistent<float>
{
public:
  FuseDelay( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "FuseDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent
#endif // __CMP_FUSEDELAY_HPP__