#ifndef __CMP_PLAYER_FOOTSTEP_FADEDELAY_HPP__
#define __CMP_PLAYER_FOOTSTEP_FADEDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class PlayerFootstepFadeDelay : public BasePersistent<float>
{
public:
  PlayerFootstepFadeDelay( float value = 5.f, float min_value = 0.1f, float max_value = 10.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerFootstepFadeDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_FOOTSTEP_FADEDELAY_HPP__
