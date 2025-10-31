#ifndef __CMP_NPC_LERP_SPEED_HPP__
#define __CMP_NPC_LERP_SPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcLerpSpeed : public BasePersistent<float>
{
public:
  NpcLerpSpeed( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcLerpSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_LERP_SPEED_HPP__