#ifndef __CMP_NPC_SCAN_SCALE_HPP__
#define __CMP_NPC_SCAN_SCALE_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NPCScanScale : public BasePersistent<float>
{
public:
  NPCScanScale()
      : BasePersistent<float>( 2.5f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_SCAN_SCALE_HPP__
