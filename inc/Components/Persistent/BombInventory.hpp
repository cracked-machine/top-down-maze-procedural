#ifndef __CMP_BOMBINVENTORY_HPP__
#define __CMP_BOMBINVENTORY_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class BombInventory
{
public:
  int &operator()() { return bomb_count; }

private:
  int bomb_count = 10;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BOMBINVENTORY_HPP__
