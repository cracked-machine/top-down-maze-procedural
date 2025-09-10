#ifndef __CMP_NPC_LERP_SPEED_HPP__
#define __CMP_NPC_LERP_SPEED_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class NpcLerpSpeed
{
public:
  float &operator()() { return value; }

private:
  float value{ 1.f };
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_LERP_SPEED_HPP__