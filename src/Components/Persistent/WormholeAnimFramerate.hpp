#ifndef SRC_COMPONENTS_PERSISTENT_WORMHOLEANIMFRAMERATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_WORMHOLEANIMFRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace Game::Cmp::Persist
{

//! @brief Frame duration of the wormhole animation.
class WormholeAnimFramerate : public BasePersistent<float>
{
public:
  //! @brief Construct with an initial frame duration and optional min/max bounds.
  //! @note Time unit is seconds per frame; the engine converts to sf::Time as needed.
  //! @param seconds_per_frame initial frame duration in seconds per frame
  //! @param min_value minimum allowed frame duration
  //! @param max_value maximum allowed frame duration
  WormholeAnimFramerate( float seconds_per_frame = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( seconds_per_frame, min_value, max_value )
  {
  }
  //! @brief Get the class name used for persistence/registry lookup.
  //! @return std::string the class name "WormholeAnimFramerate"
  virtual std::string class_name() const override { return "WormholeAnimFramerate"; }
  //! @brief Get the human-readable detail/description text (currently unset).
  //! @return std::string empty detail string
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_WORMHOLEANIMFRAMERATE_HPP__
