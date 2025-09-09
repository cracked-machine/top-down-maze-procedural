#ifndef __CMP_PLAYERDAMAGE_HPP__
#define __CMP_PLAYERDAMAGE_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class PlayerDamage
{
public:
  int &operator()() { return damage; }

private:
  int damage = 10;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYERDAMAGE_HPP__