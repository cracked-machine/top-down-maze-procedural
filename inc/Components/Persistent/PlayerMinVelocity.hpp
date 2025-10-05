#ifndef __CMP_PLAYERMINVELOCITY_HPP__
#define __CMP_PLAYERMINVELOCITY_HPP__

#include <Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

// Used to clamp player velocity to zero when very low
// Prevents jittery movement when friction is applied
// See CollisionSystem.cpp and PlayerSystem.cpp for usage
struct PlayerMinVelocity : public BasePersistent<float>
{
  PlayerMinVelocity( float value = 1.f )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYERMINVELOCITY_HPP__