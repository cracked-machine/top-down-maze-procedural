#ifndef _INC_PERSISTENT_SYSTEM_HPP
#define _INC_PERSISTENT_SYSTEM_HPP

#include <BaseSystem.hpp>

namespace ProceduralMaze::Sys {

class PersistentSystem : public BaseSystem
{
public:
  PersistentSystem( SharedEnttRegistry reg );

  void loadState();

  void saveState();
};

} // namespace ProceduralMaze::Sys

#endif // _INC_PERSISTENT_SYSTEM_HPP