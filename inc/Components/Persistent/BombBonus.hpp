#ifndef __CMP_BOMBBONUS_HPP__
#define __CMP_BOMBBONUS_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombBonus : public BasePersistent<int>
{
public:
  BombBonus( int value = 5 )
      : BasePersistent<int>( value )
  {
  }

  virtual std::string class_name() const override { return "BombBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBBONUS_HPP__