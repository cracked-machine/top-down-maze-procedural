#ifndef __CMP_BASEPERSISTENT_HPP__
#define __CMP_BASEPERSISTENT_HPP__

#include <Components/Persistent/IBasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

// Base class for persistent settings
// These types should be added to the registry as context variables
// If you add them as components to entities, they will be destroyed on game reset
// and lose their values
template <typename T>
class BasePersistent : public IBasePersistent
{
public:
  BasePersistent( T value_in, T min_value_in = T{}, T max_value_in = T{} )
      : value( value_in ),
        min_value( min_value_in ),
        max_value( max_value_in )
  {
  }

  //! @brief polymorphic destructor for derived classes
  virtual ~BasePersistent() = default;

  T &get_value() { return value; };
  T &get_min_value() { return min_value; };
  T &get_max_value() { return max_value; };

  // object serialization to json
  virtual nlohmann::json serialize() const override
  {
    nlohmann::json json_data;

    if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
    {
      json_data["type"] = "float";
      json_data["value"] = value;
      json_data["min_value"] = min_value;
      json_data["max_value"] = max_value;
    }
    else if constexpr ( std::is_integral_v<T> && !std::is_same_v<T, bool> )
    {
      json_data["type"] = "int";
      json_data["value"] = value;
      json_data["min_value"] = min_value;
      json_data["max_value"] = max_value;
    }
    else if constexpr ( std::is_same_v<T, bool> )
    {
      json_data["type"] = "bool";
      json_data["value"] = value;
      json_data["min_value"] = min_value;
      json_data["max_value"] = max_value;
    }
    else if constexpr ( std::is_same_v<T, std::string> )
    {
      json_data["type"] = "string";
      json_data["value"] = value;
      json_data["min_value"] = min_value;
      json_data["max_value"] = max_value;
    }

    return json_data;
  }

  // json deserialization to object
  virtual void deserialize( const nlohmann::json &json_data ) override
  {
    if ( json_data.contains( "value" ) )
    {
      if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
      {
        if ( json_data["value"].is_number() ) { value = json_data["value"].get<T>(); }
      }
      else if constexpr ( std::is_integral_v<T> && !std::is_same_v<T, bool> )
      {
        if ( json_data["value"].is_number_integer() ) { value = json_data["value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, bool> )
      {
        if ( json_data["value"].is_boolean() ) { value = json_data["value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, std::string> )
      {
        if ( json_data["value"].is_string() ) { value = json_data["value"].get<T>(); }
      }
    }

    if ( json_data.contains( "min_value" ) )
    {
      if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
      {
        if ( json_data["min_value"].is_number() ) { min_value = json_data["min_value"].get<T>(); }
      }
      else if constexpr ( std::is_integral_v<T> && !std::is_same_v<T, bool> )
      {
        if ( json_data["min_value"].is_number_integer() ) { min_value = json_data["min_value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, bool> )
      {
        if ( json_data["min_value"].is_boolean() ) { min_value = json_data["min_value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, std::string> )
      {
        if ( json_data["min_value"].is_string() ) { min_value = json_data["min_value"].get<T>(); }
      }
    }

    if ( json_data.contains( "max_value" ) )
    {
      if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
      {
        if ( json_data["max_value"].is_number() ) { max_value = json_data["max_value"].get<T>(); }
      }
      else if constexpr ( std::is_integral_v<T> && !std::is_same_v<T, bool> )
      {
        if ( json_data["max_value"].is_number_integer() ) { max_value = json_data["max_value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, bool> )
      {
        if ( json_data["max_value"].is_boolean() ) { max_value = json_data["max_value"].get<T>(); }
      }
      else if constexpr ( std::is_same_v<T, std::string> )
      {
        if ( json_data["max_value"].is_string() ) { max_value = json_data["max_value"].get<T>(); }
      }
    }
  }

protected:
  T value{};
  T min_value{};
  T max_value{};
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BASEPERSISTENT_HPP__
