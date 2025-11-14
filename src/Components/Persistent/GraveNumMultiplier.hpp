#ifndef __CMP_GRAVENUMMULTIPLIER_HPP__
#define __CMP_GRAVENUMMULTIPLIER_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class GraveNumMultiplier : public BasePersistent<unsigned int>
{
public:
  GraveNumMultiplier( unsigned int value = 30, unsigned int min_value = 20, unsigned int max_value = 100 )
      : BasePersistent<unsigned int>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "GraveNumMultiplier"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_GRAVENUMMULTIPLIER_HPP__
