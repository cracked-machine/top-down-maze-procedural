#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcDamage : public BasePersistent<int>
{
public:
  NpcDamage() : BasePersistent<int>( 10 ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCDAMAGE_HPP__
