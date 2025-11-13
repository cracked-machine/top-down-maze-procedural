#ifndef __CMP_CORRUPTIONDAMAGE_HPP__
#define __CMP_CORRUPTIONDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class CorruptionDamage : public BasePersistent<int>
{
public:
  CorruptionDamage( int value = 1, int min_value = 0, int max_value = 10 )
      : BasePersistent<int>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "CorruptionDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_CORRUPTIONDAMAGE_HPP__