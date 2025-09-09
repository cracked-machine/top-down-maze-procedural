#ifndef __CMP_BOMBBONUS_HPP__
#define __CMP_BOMBBONUS_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class BombBonus
{
public:
  int &operator()() { return value; }

private:
  int value = 5;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBBONUS_HPP__
