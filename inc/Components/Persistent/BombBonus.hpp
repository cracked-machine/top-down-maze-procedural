#ifndef __CMP_BOMBBONUS_HPP__
#define __CMP_BOMBBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombBonus : public BasePersistent<int>
{
public:
  BombBonus() : BasePersistent<int>( 5 ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBBONUS_HPP__