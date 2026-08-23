#ifndef SRC_COMPONENTS_PERSISTENT_PLAYERANIMFRAMERATE_HPP__
#define SRC_COMPONENTS_PERSISTENT_PLAYERANIMFRAMERATE_HPP__

#include <Components/Persistent/BasePersistent.hpp>
#include <SFML/System/Time.hpp>

namespace Game::Cmp::Persist
{

//! @brief Persistent setting for the player's sprite animation framerate.
class PlayerAnimFramerate : public BasePersistent<float>
{
public:
  //! @brief Construct a new PlayerAnimFramerate object.
  //! @note The time unit is seconds per frame; the engine should convert to sf::Time as needed.
  //! @param seconds_per_frame Initial framerate value, expressed as seconds per frame.
  //! @param min_value Minimum allowed value.
  //! @param max_value Maximum allowed value.
  PlayerAnimFramerate( float seconds_per_frame = 0.f, float min_value = 0.f, float max_value = 0.f )
      : BasePersistent<float>( seconds_per_frame, min_value, max_value )
  {
  }
  //! @brief Returns the class name used for identification and JSON serialization.
  //! @return std::string The class name.
  virtual std::string class_name() const override { return "PlayerAnimFramerate"; }
  //! @brief Returns the display detail string for this setting (currently unused/empty).
  //! @return std::string The detail string.
  const std::string get_detail() const override { return ""; }
};

} // namespace Game::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_PLAYERANIMFRAMERATE_HPP__
