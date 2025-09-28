#ifndef __CMP_PLAYER_LERP_SPEED_HPP__
#define __CMP_PLAYER_LERP_SPEED_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class PlayerLerpSpeed : public BasePersistent<float>
{
public:
  PlayerLerpSpeed()
      : BasePersistent<float>( 5.f )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_LERP_SPEED_HPP__