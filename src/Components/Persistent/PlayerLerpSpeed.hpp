#ifndef __CMP_PLAYER_LERP_SPEED_HPP__
#define __CMP_PLAYER_LERP_SPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerLerpSpeed : public BasePersistent<float>
{
public:
  PlayerLerpSpeed( float value = 4.0, float min_value = 3.f, float max_value = 10.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerLerpSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_LERP_SPEED_HPP__