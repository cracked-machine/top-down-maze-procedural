#ifndef __CMP_ARMEDONDELAY_HPP__
#define __CMP_ARMEDONDELAY_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being armed
class ArmedOnDelay : public BasePersistent<float>
{
public:
  ArmedOnDelay()
      : BasePersistent<float>( 0.025f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_ARMEDONDELAY_HPP__