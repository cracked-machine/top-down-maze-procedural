#ifndef __CMP_CORRUPTIONSEED_HPP__
#define __CMP_CORRUPTIONSEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class CorruptionSeed : public BasePersistent<unsigned long>
{
public:
  CorruptionSeed()
      : BasePersistent<unsigned long>( 0 )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_CORRUPTIONSEED_HPP__
