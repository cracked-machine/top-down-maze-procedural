#ifndef __CMP_NPC_DEATH_ANIM_FRAMERATE_HPP__
#define __CMP_NPC_DEATH_ANIM_FRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class NpcDeathAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  NpcDeathAnimFramerate( float seconds_per_frame = 0.1f )
      : BasePersistent<float>( seconds_per_frame )
  {
  }
  virtual std::string class_name() const override { return "NpcDeathAnimFramerate"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_DEATH_ANIM_FRAMERATE_HPP__