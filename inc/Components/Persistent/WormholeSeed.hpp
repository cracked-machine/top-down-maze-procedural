#ifndef __CMP_WORMHOLESEED_HPP__
#define __CMP_WORMHOLESEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class WormholeSeed : public BasePersistent<unsigned long>
{
public:
  WormholeSeed( unsigned long value = 0 )
      : BasePersistent<unsigned long>( value )
  {
  }
  virtual std::string class_name() const override { return "WormholeSeed"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WORMHOLESEED_HPP__
