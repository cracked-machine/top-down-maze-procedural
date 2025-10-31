#ifndef __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerSubmergedLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerSubmergedLerpSpeedModifier( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "PlayerSubmergedLerpSpeedModifier"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_SUBMERGED_LERP_SPEED_MOD_HPP__