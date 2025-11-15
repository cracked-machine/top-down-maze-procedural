#ifndef __CMP_GRAVENUMMULTIPLIER_HPP__
#define __CMP_GRAVENUMMULTIPLIER_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class GraveNumMultiplier : public BasePersistent<uint8_t>
{
public:
  GraveNumMultiplier( uint8_t value = 30, uint8_t min_value = 20, uint8_t max_value = 100 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "GraveNumMultiplier"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_GRAVENUMMULTIPLIER_HPP__
