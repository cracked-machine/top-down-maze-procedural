#ifndef __CMP_BOMBDAMAGE_HPP__
#define __CMP_BOMBDAMAGE_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class BombDamage
{
public:
  int &operator()() { return value; }

private:
  int value = 10;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBDAMAGE_HPP__