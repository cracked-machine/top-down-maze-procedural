#ifndef __CMP_BOMBBONUS_HPP__
#define __CMP_BOMBBONUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombBonus : public BasePersistent<int>
{
public:
  BombBonus( int value = 3, int min_value = 0, int max_value = 20 )
      : BasePersistent<int>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BombBonus"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBBONUS_HPP__