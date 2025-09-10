#ifndef __CMP_FLOODSPEED_HPP__
#define __CMP_FLOODSPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class FloodSpeed : public BasePersistent<float>
{
public:
  FloodSpeed() : BasePersistent<float>( 4.f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FLOODSPEED_HPP__