#ifndef __CMP_NPC_ACTIVATE_SCALE_HPP__
#define __CMP_NPC_ACTIVATE_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class NpcActivateScale : public BasePersistent<float>
{
public:
  NpcActivateScale( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcActivateScale"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPC_ACTIVATE_SCALE_HPP__
