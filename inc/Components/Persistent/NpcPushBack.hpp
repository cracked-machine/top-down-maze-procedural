#ifndef __CMP_NPCPUSHBACK_HPP__
#define __CMP_NPCPUSHBACK_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class NpcPushBack
{
public:
  float &operator()() { return value; }

private:
  float value = 8.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCPUSHBACK_HPP__
