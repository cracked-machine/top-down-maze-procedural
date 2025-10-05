#ifndef __CMP_NPC_LERP_SPEED_HPP__
#define __CMP_NPC_LERP_SPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcLerpSpeed : public BasePersistent<float>
{
public:
  NpcLerpSpeed( float value = 1.f )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_LERP_SPEED_HPP__