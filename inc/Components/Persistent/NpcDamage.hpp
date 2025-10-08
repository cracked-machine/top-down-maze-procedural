#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcDamage : public BasePersistent<int>
{
public:
  NpcDamage( int value = 10 )
      : BasePersistent<int>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCDAMAGE_HPP__
