#ifndef __CMP_NPCPUSHBACK_HPP__
#define __CMP_NPCPUSHBACK_HPP__

#include <BaseSystem.hpp>
#include <MultiSprite.hpp>
#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcPushBack : public BasePersistent<float>
{
public:
  NpcPushBack( float value )
      : BasePersistent<float>( value )
  {
  }
  virtual std::string class_name() const override { return "NpcPushBack"; }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCPUSHBACK_HPP__
