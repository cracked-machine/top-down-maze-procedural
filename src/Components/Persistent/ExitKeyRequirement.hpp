#ifndef SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP__
#define SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief The number of keys the player must hold to unlock the exit
//!
class ExitKeyRequirement : public BasePersistent<uint8_t>
{
public:
  ExitKeyRequirement( uint8_t value = 0, uint8_t min_value = 0, uint8_t max_value = 0 )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  virtual std::string class_name() const override { return "ExitKeyRequirement"; }
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_EXITKEYREQUIREMENT_HPP__
