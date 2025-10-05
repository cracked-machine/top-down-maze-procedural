#ifndef __CMP_NPCPUSHBACK_HPP__
#define __CMP_NPCPUSHBACK_HPP__

#include <MultiSprite.hpp>
#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class NpcPushBack : public BasePersistent<float>
{
public:
  NpcPushBack( float value = ProceduralMaze::Sprites::MultiSprite::kDefaultSpriteDimensions.x )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_NPCPUSHBACK_HPP__
