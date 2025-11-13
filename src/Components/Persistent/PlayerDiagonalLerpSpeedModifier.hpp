#ifndef __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerDiagonalLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerDiagonalLerpSpeedModifier( float value = 0.707, float min_value = 0.001f, float max_value = 1.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDiagonalLerpSpeedModifier"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__