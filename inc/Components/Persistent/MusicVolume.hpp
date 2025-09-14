#ifndef __CMP_MUSICVOLUME_HPP__
#define __CMP_MUSICVOLUME_HPP__

#include <Persistent/BasePersistent.hpp>
namespace ProceduralMaze::Cmp::Persistent {

class MusicVolume : public BasePersistent<float>
{
public:
  MusicVolume() : BasePersistent<float>( 100.0f ) {}
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_MUSICVOLUME_HPP__
