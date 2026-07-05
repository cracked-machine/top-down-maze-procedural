#ifndef SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_
#define SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

struct ArmedBlinkFreq : public BasePersistent<float>
{
  ArmedBlinkFreq( float freq = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( freq, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "ArmedBlinkFreq"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_ARMED_CMPS_PERSIST_ARMEDBLINKFREQ_HPP_