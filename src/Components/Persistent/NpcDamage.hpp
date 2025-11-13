#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcDamage : public BasePersistent<int>
{
public:
  NpcDamage( int value = 15, int min_value = 1, int max_value = 50 )
      : BasePersistent<int>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCDAMAGE_HPP__
