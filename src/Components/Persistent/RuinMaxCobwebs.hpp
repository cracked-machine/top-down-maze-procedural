#ifndef SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <cstdint>

namespace Game::Cmp::Persist
{

//! @brief Persistent setting for the maximum number of cobwebs that can be procedurally spawned
//! in a ruin.
class RuinMaxCobwebs : public BasePersistent<uint16_t>
{
public:
  //! @brief Construct a new RuinMaxCobwebs object.
  //! @param value Initial maximum cobweb count.
  //! @param min_value Minimum allowed value.
  //! @param max_value Maximum allowed value.
  RuinMaxCobwebs( uint16_t value = 0, uint16_t min_value = 0, uint16_t max_value = 0 )
      : BasePersistent<uint16_t>( value, min_value, max_value )
  {
  }

  //! @brief Returns the class name used for identification and JSON serialization.
  //! @return std::string The class name.
  [[nodiscard]] std::string class_name() const override { return "RuinMaxCobwebs"; }
  //! @brief Returns the display detail string for this setting (currently unused/empty).
  //! @return std::string The detail string.
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINMAXCOBWEBS_HPP__
