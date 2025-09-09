#ifndef __CMP_FLOODSPEED_HPP__
#define __CMP_FLOODSPEED_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class FloodSpeed
{
public:
  float &operator()() { return speed; }

private:
  float speed{ 4.f };
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FLOODSPEED_HPP__