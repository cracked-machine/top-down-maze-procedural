#ifndef __CMP_PLAYERMAXSPEED_HPP__
#define __CMP_PLAYERMAXSPEED_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class PlayerMaxSpeed
{
public:
  float &operator()() { return max_speed; }

private:
  float max_speed = 100.f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_PLAYERMAXSPEED_HPP__
