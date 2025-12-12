#ifndef __CMP_PLAYER_ANIM_FRAMERATE_HPP__
#define __CMP_PLAYER_ANIM_FRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class PlayerAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  PlayerAnimFramerate( float seconds_per_frame = 0.1, float min_value = 0.01f,
                       float max_value = 0.5f )
      : BasePersistent<float>( seconds_per_frame, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "PlayerAnimFramerate"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_PLAYER_ANIM_FRAMERATE_HPP__