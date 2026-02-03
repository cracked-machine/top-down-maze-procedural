#ifndef __CMP_PLAYER_LERP_SPEED_HPP__
#define __CMP_PLAYER_LERP_SPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class PlayerLerpSpeed : public BasePersistent<float>
{
public:
  PlayerLerpSpeed( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerLerpSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PLAYER_LERP_SPEED_HPP__