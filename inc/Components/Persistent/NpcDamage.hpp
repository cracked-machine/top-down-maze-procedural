#ifndef __CMP_NPCDAMAGE_HPP__
#define __CMP_NPCDAMAGE_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class NpcDamage
{
public:
  int &operator()() { return value; }

private:
  int value = 10;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCDAMAGE_HPP__