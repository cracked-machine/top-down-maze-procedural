#ifndef __CMP_NPC_ANIM_FRAMERATE_HPP__
#define __CMP_NPC_ANIM_FRAMERATE_HPP__

#include <Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class NpcAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  NpcAnimFramerate( float seconds_per_frame = 0.1f )
      : BasePersistent<float>( seconds_per_frame )
  {
  }
  virtual std::string class_name() const override { return "NpcAnimFramerate"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYER_ANIM_FRAMERATE_HPP__