#ifndef __CMP_NPC_SCAN_SCALE_HPP__
#define __CMP_NPC_SCAN_SCALE_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class NPCScanScale
{
public:
  float &operator()() { return value; }

private:
  float value{ 2.5f };
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPC_SCAN_SCALE_HPP__