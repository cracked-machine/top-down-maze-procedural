#ifndef SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__
#define SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Delay between each concentric obstacle in the bomb pattern being armed.
class CryptShuffleTimeout : public BasePersistent<float>
{
public:
  //! @brief Construct with an initial timeout value and optional min/max bounds.
  //! @param value initial timeout value, in seconds
  //! @param min_value minimum allowed timeout value
  //! @param max_value maximum allowed timeout value
  CryptShuffleTimeout( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }

  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "CryptShuffleTimeout"
  [[nodiscard]] std::string class_name() const override { return "CryptShuffleTimeout"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__
