#ifndef __CMP_BLASTRADIUS_HPP__
#define __CMP_BLASTRADIUS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>
namespace ProceduralMaze::Cmp::Persist
{

class BlastRadius : public BasePersistent<uint8_t>
{
public:
  BlastRadius( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }

  virtual std::string class_name() const override { return "BlastRadius"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_BLASTRADIUS_HPP__
