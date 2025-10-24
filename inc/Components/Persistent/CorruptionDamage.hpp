#ifndef __CMP_CORRUPTIONDAMAGE_HPP__
#define __CMP_CORRUPTIONDAMAGE_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class CorruptionDamage : public BasePersistent<int>
{
public:
  CorruptionDamage( int value )
      : BasePersistent<int>( value )
  {
  }
  virtual std::string class_name() const override { return "CorruptionDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_CORRUPTIONDAMAGE_HPP__