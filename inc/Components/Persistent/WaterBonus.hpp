#ifndef __CMP_WATERBONUS_HPP__
#define __CMP_WATERBONUS_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class WaterBonus
{
public:
  float &operator()() { return value; }

private:
  float value = 100.0f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_WATERBONUS_HPP__
