#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class NpcDamage : public BasePersistent<uint8_t>
{
public:
  NpcDamage( uint8_t value = 15, uint8_t min_value = 1, uint8_t max_value = 50 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCDAMAGE_HPP__
