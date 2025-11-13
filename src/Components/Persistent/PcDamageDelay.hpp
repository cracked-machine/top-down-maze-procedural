#ifndef __CMP_PC_DAMAGE_DELAY_HPP__
#define __CMP_PC_DAMAGE_DELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PcDamageDelay : public BasePersistent<float>
{
public:
  PcDamageDelay( float value = 3.0f, float min_value = 0.1f, float max_value = 2.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PcDamageDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PC_DAMAGE_DELAY_HPP__
