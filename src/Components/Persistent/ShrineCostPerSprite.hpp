#ifndef __CMP_SHRINECOST_HPP__
#define __CMP_SHRINECOST_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

//! @brief Shrine consists of 4 sprites, so this is cost per sprite.
//!
class ShrineCostPerSprite : public BasePersistent<unsigned int>
{
public:
  ShrineCostPerSprite( unsigned int value = 1, unsigned int min_value = 0, unsigned int max_value = 4 )
      : BasePersistent<unsigned int>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "ShrineCostPerSprite"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_SHRINECOST_HPP__
