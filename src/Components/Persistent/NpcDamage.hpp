#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcDamage : public BasePersistent<uint8_t>
{
public:
  NpcDamage( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcDamage"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPCDAMAGE_HPP__
