#ifndef __CMP_MAXNUMALTARS_HPP__
#define __CMP_MAXNUMALTARS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class MaxNumAltars : public BasePersistent<uint8_t>
{
public:
  MaxNumAltars( uint8_t value = 3, uint8_t min_value = 1, uint8_t max_value = 10 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MaxNumAltars"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MAXNUMALTARS_HPP__
