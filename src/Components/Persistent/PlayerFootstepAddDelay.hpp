#ifndef __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__
#define __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class PlayerFootstepAddDelay : public BasePersistent<float>
{
public:
  PlayerFootstepAddDelay( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerFootstepAddDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__
