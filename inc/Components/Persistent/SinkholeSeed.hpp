#ifndef __CMP_SINKHOLESEED_HPP__
#define __CMP_SINKHOLESEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class SinkholeSeed : public BasePersistent<unsigned long>
{
public:
  SinkholeSeed( float value = 0 )
      : BasePersistent<unsigned long>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_SINKHOLESEED_HPP__
