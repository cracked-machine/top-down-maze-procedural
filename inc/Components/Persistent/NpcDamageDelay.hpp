#ifndef __CMP_NPC_DAMAGE_DELAY_HPP__
#define __CMP_NPC_DAMAGE_DELAY_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class NpcDamageDelay : public BasePersistent<float>
{
public:
  NpcDamageDelay() : BasePersistent<float>( 0.5f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_DAMAGE_DELAY_HPP__
