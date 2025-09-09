#ifndef __CMP_FRICTIONCOEFFICIENT_HPP__
#define __CMP_FRICTIONCOEFFICIENT_HPP__

namespace ProceduralMaze::Cmp::Persistent {

class FrictionCoefficient
{
public:
  float &operator()() { return friction_coefficient; }

private:
  float friction_coefficient = 0.02f;
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_FRICTIONCOEFFICIENT_HPP__
