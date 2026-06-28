#ifndef SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__
#define SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__

#include <Components/Persistent/BasePersistent.hpp>

namespace Game::Cmp::Persist
{

// Delay between each concentric obstacle in bomb pattern being armed
class CryptShuffleTimeout : public BasePersistent<float>
{
public:
  CryptShuffleTimeout( float value = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( value, min_value, max_value )
  {
  }

  [[nodiscard]] std::string class_name() const override { return "CryptShuffleTimeout"; }
  [[nodiscard]] const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_CRYPTSHUFFLETIMEOUT_HPP__
