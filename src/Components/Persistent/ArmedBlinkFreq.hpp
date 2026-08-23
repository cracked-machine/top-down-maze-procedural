#ifndef SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_
#define SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

//! @brief Frequency (Hz) at which armed bomb obstacle blocks blink to warn the player.
struct ArmedBlinkFreq : public BasePersistent<float>
{
  //! @brief Construct with an initial blink frequency and optional min/max bounds.
  //! @param freq initial blink frequency
  //! @param min_value minimum allowed frequency
  //! @param max_value maximum allowed frequency
  ArmedBlinkFreq( float freq = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( freq, min_value, max_value )
  {
  }

  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "ArmedBlinkFreq"
  [[nodiscard]] std::string class_name() const override { return "ArmedBlinkFreq"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_