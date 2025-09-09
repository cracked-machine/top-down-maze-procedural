#ifndef __CMP_ARMEDONDELAY_HPP__
#define __CMP_ARMEDONDELAY_HPP__

namespace ProceduralMaze::Cmp::Persistent {

// Delay between each concentric obstacle in bomb pattern being armed
class ArmedOnDelay
{
public:
  float &operator()() { return delay; }

private:
  float delay = 0.025f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_ARMEDONDELAY_HPP__