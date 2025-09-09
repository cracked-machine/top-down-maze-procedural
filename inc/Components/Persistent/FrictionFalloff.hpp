#ifndef __CMP_FRICTIONFALLOFF_HPP__
#define __CMP_FRICTIONFALLOFF_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class FrictionFalloff
{
public:
  float &operator()() { return friction_falloff; }

private:
  float friction_falloff = 0.5f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FRICTIONFALLOFF_HPP__
