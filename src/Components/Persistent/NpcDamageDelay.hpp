#ifndef __CMP_NPC_DAMAGE_DELAY_HPP__
#define __CMP_NPC_DAMAGE_DELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcDamageDelay : public BasePersistent<float>
{
public:
  NpcDamageDelay( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcDamageDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_DAMAGE_DELAY_HPP__
