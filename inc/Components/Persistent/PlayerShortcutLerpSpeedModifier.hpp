#ifndef __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__
#define __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerShortcutLerpSpeedModifier : public BasePersistent<float>
{
public:
  PlayerShortcutLerpSpeedModifier()
      : BasePersistent<float>( 0.3f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_SHORTCUT_LERP_SPEED_MOD_HPP__