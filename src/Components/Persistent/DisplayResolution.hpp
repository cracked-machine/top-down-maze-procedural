#ifndef SRC_COMPONENTS_PERSISTENT_DISPLAYRESOLUTION_HPP__
#define SRC_COMPONENTS_PERSISTENT_DISPLAYRESOLUTION_HPP__

#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <string>
namespace ProceduralMaze::Cmp::Persist
{

class DisplayResolution : public sf::Vector2u
{
public:
  DisplayResolution() = default;
  DisplayResolution( unsigned int width, unsigned int height )
      : sf::Vector2u( width, height ) {};
  DisplayResolution( sf::Vector2u vec )
      : sf::Vector2u( vec ) {};

  std::string class_name() const { return "DisplayResolution"; }

  std::string &get_value()
  {
    std::stringstream ss;
    ss << "x: " << x << " y: " << y;
    static std::string str = ss.str();
    return str;
  };

  void deserialize( const nlohmann::json &json_data )
  {
    if ( json_data.contains( "value" ) && json_data["value"].is_object() )
    {
      unsigned int new_x = json_data["value"].value( "x", 0u );
      unsigned int new_y = json_data["value"].value( "y", 0u );

      // we cannot use 0,0 as a resolution, so clamp to a fallback default,
      // use a silly low resolution so its obvious that something is wrong
      if ( new_x < 800 && new_y < 600 )
      {
        x = 800;
        y = 600;
      }
      else
      {
        x = new_x;
        y = new_y;
      }
    }
  }

  virtual nlohmann::json serialize() const
  {
    nlohmann::json json_data;
    json_data["type"] = "sf::Vector2u";
    json_data["value"]["x"] = x;
    json_data["value"]["y"] = y;
    return json_data;
  }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // SRC_COMPONENTS_PERSISTENT_DISPLAYRESOLUTION_HPP__