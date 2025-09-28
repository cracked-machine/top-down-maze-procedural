#ifndef __CMP_NPCPUSHBACK_HPP__
#define __CMP_NPCPUSHBACK_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcPushBack : public BasePersistent<float>
{
public:
  NpcPushBack()
      : BasePersistent<float>( 16.0f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCPUSHBACK_HPP__
