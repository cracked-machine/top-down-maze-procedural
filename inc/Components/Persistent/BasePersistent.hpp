#ifndef __CMP_BASEPERSISTENT_HPP__
#define __CMP_BASEPERSISTENT_HPP__

#include <Persistent/IBasePersistent.hpp>

namespace ProceduralMaze::Cmp::Persistent {

// Base class for persistent settings
// These types should be added to the registry as context variables
// If you add them as components to entities, they will be destroyed on game reset
// and lose their values
template <typename T> class BasePersistent : public IBasePersistent
{
public:
  BasePersistent( T value_in )
      : value( value_in )
  {
  }
  virtual ~BasePersistent() = default;
  T &operator()() { return value; };

  // object serialization to json
  virtual nlohmann::json serialize() const override
  {
    nlohmann::json json_data;

    if constexpr ( std::is_same_v<T, float> || std::is_same_v<T, double> )
    {
      json_data["type"] = "float";
      json_data["value"] = value;
    }
    else if constexpr ( std::is_integral_v<T> && !std::is_same_v<T, bool> )
    {
      json_data["type"] = "int";
      json_data["value"] = value;
    }
    else if constexpr ( std::is_same_v<T, bool> )
    {
      json_data["type"] = "bool";
      json_data["value"] = value;
    }
    else if constexpr ( std::is_same_v<T, std::string> )
    {
      json_data["type"] = "string";
      json_data["value"] = value;
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
  }

protected:
  T value{};
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __CMP_BASEPERSISTENT_HPP__
