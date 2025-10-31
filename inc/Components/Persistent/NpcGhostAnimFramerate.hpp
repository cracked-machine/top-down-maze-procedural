#ifndef __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__
#define __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__

#include <Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace ProceduralMaze::Cmp::Persistent {

class NpcGhostAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  NpcGhostAnimFramerate( float seconds_per_frame )
      : BasePersistent<float>( seconds_per_frame )
  {
  }
  virtual std::string class_name() const override { return "NpcGhostAnimFramerate"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__