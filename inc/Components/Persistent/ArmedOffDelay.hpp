#ifndef __CMP__ARMEDOFFDELAY_HPP__
#define __CMP__ARMEDOFFDELAY_HPP__

namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being disarmed...violently
class ArmedOffDelay
{
public:
  float &operator()() { return delay; }

private:
  float delay = 0.075f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP__ARMEDOFFDELAY_HPP__