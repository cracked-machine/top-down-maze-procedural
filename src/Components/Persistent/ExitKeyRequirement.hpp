#ifndef SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP_
#define SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP_

#include <Components/Persistent/BasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persist
{

//! @brief The number of keys the player must hold to unlock the exit
//!
class ExitKeyRequirement : public BasePersistent<uint8_t>
{
public:
  ExitKeyRequirement( uint8_t value = 3, uint8_t min_value = 1, uint8_t max_value = 10 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "ExitKeyRequirement"; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP_
