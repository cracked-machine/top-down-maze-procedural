#ifndef __CMP_MAXSHRINES_HPP__
#define __CMP_MAXSHRINES_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class MaxShrines : public BasePersistent<unsigned int>
{
public:
  MaxShrines( unsigned int value = 3, unsigned int min_value = 1, unsigned int max_value = 10 )
      : BasePersistent<unsigned int>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "MaxShrines"; }

};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MAXSHRINES_HPP__
