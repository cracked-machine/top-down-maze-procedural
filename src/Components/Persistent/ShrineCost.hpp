#ifndef __CMP_SHRINECOST_HPP__
#define __CMP_SHRINECOST_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class ShrineCost : public BasePersistent<unsigned int>
{
public:
  ShrineCost( unsigned int value = 1 )
      : BasePersistent<unsigned int>( value )
  {
  }
  virtual std::string class_name() const override { return "ShrineCost"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_SHRINECOST_HPP__
