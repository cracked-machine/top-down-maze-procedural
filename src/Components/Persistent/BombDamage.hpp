#ifndef __CMP_BOMBDAMAGE_HPP__
#define __CMP_BOMBDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class BombDamage : public BasePersistent<int>
{
public:
  BombDamage( int value = 10 )
      : BasePersistent<int>( value )
  {
  }
  virtual std::string class_name() const override { return "BombDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBDAMAGE_HPP__