#ifndef __CMP_NPC_LERP_SPEED_HPP__
#define __CMP_NPC_LERP_SPEED_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcLerpSpeed : public BasePersistent<float>
{
public:
  NpcLerpSpeed( float value = 1.0f, float min_value = 0.1f, float max_value = 3.0f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcLerpSpeed"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPC_LERP_SPEED_HPP__