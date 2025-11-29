#ifndef __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__
#define __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent
{

class PlayerFootstepAddDelay : public BasePersistent<float>
{
public:
  PlayerFootstepAddDelay( float value = 0.2, float min_value = 0.01f, float max_value = 1.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerFootstepAddDelay"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_FOOTSTEP_ADDDELAY_HPP__
