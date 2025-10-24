#ifndef __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerDiagonalLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerDiagonalLerpSpeedModifier( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "PlayerDiagonalLerpSpeedModifier"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_DIAGONAL_LERP_SPEED_MOD_HPP__