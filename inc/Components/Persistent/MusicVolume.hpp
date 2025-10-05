#ifndef __CMP_MUSICVOLUME_HPP__
#define __CMP_MUSICVOLUME_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class MusicVolume : public BasePersistent<float>
{
public:
  MusicVolume( float value = 100.0f )
      : BasePersistent<float>( value )
  {
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MUSICVOLUME_HPP__
