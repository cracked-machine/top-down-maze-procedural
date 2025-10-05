#ifndef __CMP_NPC_ACTIVATE_SCALE_HPP__
#define __CMP_NPC_ACTIVATE_SCALE_HPP__

#include <Persistent/BasePersistent.hpp>
#include <RectBounds.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NPCActivateScale : public BasePersistent<float>
{
public:
  NPCActivateScale( float value = 5.f )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_ACTIVATE_SCALE_HPP__
