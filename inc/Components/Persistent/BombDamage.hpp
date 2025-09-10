#ifndef __CMP_BOMBDAMAGE_HPP__
#define __CMP_BOMBDAMAGE_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombDamage : public BasePersistent<int>
{
public:
  BombDamage() : BasePersistent<int>( 10 ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBDAMAGE_HPP__