#ifndef __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerSubmergedLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerSubmergedLerpSpeedModifier()
      : BasePersistent<float>( 0.707f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__