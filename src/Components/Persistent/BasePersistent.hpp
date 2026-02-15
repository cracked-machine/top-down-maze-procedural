#ifndef __CMP_BASEPERSISTENT_HPP__
#define __CMP_BASEPERSISTENT_HPP__

#include <Components/Persistent/IBasePersistent.hpp>

#include <nlohmann/json_fwd.hpp>
#include <source_location>

namespace ProceduralMaze::Cmp::Persist
{

// Logging helpers - defined in BasePersistent.cpp
void log_debug_msg( const std::string &msg, const std::source_location &loc = std::source_location::current() );
void log_warn_msg( const std::string &msg, const std::source_location &loc = std::source_location::current() );

// Base class for persistent settings
// These types should be added to the registry as context variables
// If you add them as components to entities, they will be destroyed on game reset
// and lose their values
template <typename T>
class BasePersistent : public IBasePersistent
{
public:
  BasePersistent( T value_in, T min_value_in = T{}, T max_value_in = T{} );

  //! @brief polymorphic destructor for derived classes
  virtual ~BasePersistent() = default;

  T &get_value() { return value; };
  T &get_min_value() { return min_value; };
  T &get_max_value() { return max_value; };
  std::string &get_detail() { return detail; };
  std::string &get_format() { return format; };

  // Add pointer accessors for ImGui SliderScalar compatibility
  T *get_value_ptr() { return &value; };
  T *get_min_value_ptr() { return &min_value; };
  T *get_max_value_ptr() { return &max_value; };
  std::string *get_detail_ptr() { return &detail; };
  std::string *get_format_ptr() { return &format; };

  //! @brief serialise from peristent component object to json (res/json/persistent_components.json)
  //! Declarations only - defined in .cpp
  //! @return nlohmann::json
  virtual nlohmann::json serialize() const override;

  //! @brief deserialise from json (res/json/persistent_components.json) to peristent component object
  //! Declarations only - defined in .cpp
  //! @param json_data
  virtual void deserialize( const nlohmann::json &json_data ) override;

  //! @brief call imgui widget function for data type T
  //! Declarations only - defined in .cpp
  void render_widget() override;

protected:
  T value{};
  T min_value{};
  T max_value{};
  std::string detail{ "" };
  std::string format{ "" };
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __CMP_BASEPERSISTENT_HPP__
