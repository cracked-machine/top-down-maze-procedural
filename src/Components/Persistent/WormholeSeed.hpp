#ifndef __CMP_WORMHOLESEED_HPP__
#define __CMP_WORMHOLESEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

// This component does not use JSON serialization as it is managed internally by the game engine.
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
