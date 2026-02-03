#ifndef __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class PlayerShortcutLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerShortcutLerpSpeedModifier( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerShortcutLerpSpeedModifier"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__