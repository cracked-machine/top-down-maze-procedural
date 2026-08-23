#ifndef SRC_COMPONENTS_PERSISTENT_RUINPROCGENBIRTHTHRESHOLD_HPP__
#define SRC_COMPONENTS_PERSISTENT_RUINPROCGENBIRTHTHRESHOLD_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Persistent setting for the cellular-automata "birth" neighbour-count threshold used
//! when procedurally generating ruin obstacles: an empty cell with at least this many occupied
//! neighbours becomes occupied on the next iteration.
class RuinProcGenBirthThreshold : public BasePersistent<uint8_t>
{
public:
  //! @brief Construct a new RuinProcGenBirthThreshold object.
  //! @param value Initial threshold value.
  //! @param min_value Minimum allowed value.
  //! @param max_value Maximum allowed value.
  RuinProcGenBirthThreshold( uint8_t value = 0.f, uint8_t min_value = 0.f, uint8_t max_value = 0.f )
      : BasePersistent<uint8_t>( value, min_value, max_value )
  {
  }
  //! @brief Returns the class name used for identification and JSON serialization.
  //! @return std::string The class name.
  [[nodiscard]] std::string class_name() const override { return "RuinProcGenBirthThreshold"; }
  //! @brief Returns the display detail string for this setting (currently unused/empty).
  //! @return std::string The detail string.
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_RUINPROCGENBIRTHTHRESHOLD_HPP__
