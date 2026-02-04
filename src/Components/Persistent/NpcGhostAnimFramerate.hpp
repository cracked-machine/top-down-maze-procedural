#ifndef __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__
#define __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

class NpcGhostAnimFramerate : public BasePersistent<float>
{
public:
  // time unit is seconds per frame, engine should convert to sf::Time as needed
  NpcGhostAnimFramerate( float seconds_per_frame = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( seconds_per_frame, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcGhostAnimFramerate"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPC_GHOST_ANIM_FRAMERATE_HPP__