#ifndef __CMP_MAXNUMALTARS_HPP__
#define __CMP_MAXNUMALTARS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class MaxNumAltars : public BasePersistent<uint8_t>
{
public:
  MaxNumAltars( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MaxNumAltars"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_MAXNUMALTARS_HPP__
