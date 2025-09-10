#ifndef __CMP_NPC_ACTIVATE_SCALE_HPP__
#define __CMP_NPC_ACTIVATE_SCALE_HPP__

#include <RectBounds.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NPCActivateScale
{
public:
  float &operator()() { return value; }

private:
  float value{ 5.f };
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_ACTIVATE_SCALE_HPP__