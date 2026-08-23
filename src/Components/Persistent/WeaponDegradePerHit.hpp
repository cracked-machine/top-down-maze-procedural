#ifndef SRC_COMPONENTS_PERSISTENT_WEAPONDEGRADEPERHIT_HPP__
#define SRC_COMPONENTS_PERSISTENT_WEAPONDEGRADEPERHIT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Amount of wear/durability a weapon loses each time it lands a hit.
struct WeaponDegradePerHit : public BasePersistent<float>
{
  //! @brief Construct with an initial degrade amount and optional min/max bounds.
  //! @param value initial degrade amount
  //! @param min_value minimum allowed degrade amount
  //! @param max_value maximum allowed degrade amount
  WeaponDegradePerHit( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "WeaponDegradePerHit"
  virtual std::string class_name() const override { return "WeaponDegradePerHit"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_WEAPONDEGRADEPERHIT_HPP__
