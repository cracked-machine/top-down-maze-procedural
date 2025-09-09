#ifndef __CMP_HEALTHBONUS_HPP__
#define __CMP_HEALTHBONUS_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class HealthBonus
{
public:
  int &operator()() { return value; }

private:
  int value = 10;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_HEALTHBONUS_HPP__
