#ifndef __CMP_NPC_ACTIVATE_SCALE_HPP__
#define __CMP_NPC_ACTIVATE_SCALE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class NpcActivateScale : public BasePersistent<float>
{
public:
  NpcActivateScale( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcActivateScale"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_ACTIVATE_SCALE_HPP__
