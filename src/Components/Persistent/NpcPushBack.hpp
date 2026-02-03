#ifndef __CMP_NPCPUSHBACK_HPP__
#define __CMP_NPCPUSHBACK_HPP__

#include <Components/Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persist
{

class NpcPushBack : public BasePersistent<float>
{
public:
  NpcPushBack( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "NpcPushBack"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_NPCPUSHBACK_HPP__
