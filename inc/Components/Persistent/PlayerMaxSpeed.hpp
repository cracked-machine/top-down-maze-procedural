#ifndef __CMP_PLAYERMAXSPEED_HPP__
#define __CMP_PLAYERMAXSPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerMaxSpeed : public BasePersistent<float>
{
public:
  PlayerMaxSpeed() : BasePersistent<float>( 100.f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYERMAXSPEED_HPP__
