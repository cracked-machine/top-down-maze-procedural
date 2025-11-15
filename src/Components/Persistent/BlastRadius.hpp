#ifndef __CMP_BLASTRADIUS_HPP__
#define __CMP_BLASTRADIUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>
namespace ProceduralMaze::Cmp::Persistent
{

class BlastRadius : public BasePersistent<uint8_t>
{
public:
  BlastRadius( uint8_t value = 1, uint8_t min_value = 1, uint8_t max_value = 5 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BlastRadius"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BLASTRADIUS_HPP__
