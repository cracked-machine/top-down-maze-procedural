#ifndef __CMP_PLAYER_ANIM_FRAMERATE_HPP__
#define __CMP_PLAYER_ANIM_FRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class PlayerAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  PlayerAnimFramerate( float seconds_per_frame )
      : BasePersistent<float>( seconds_per_frame )
  {
  }
  virtual std::string class_name() const override { return "PlayerAnimFramerate"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_ANIM_FRAMERATE_HPP__