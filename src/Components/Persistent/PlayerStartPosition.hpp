#ifndef __COMPONENTS_PLAYERSTARTPOSITION_HPP__
#define __COMPONENTS_PLAYERSTARTPOSITION_HPP__

#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Cmp::Persistent
{

// Coordinate
class PlayerStartPosition : public sf::Vector2f
{
public:
  PlayerStartPosition( sf::Vector2f pos = { 0.0f, 0.0f } )
      : sf::Vector2f( pos )
  {
    SPDLOG_DEBUG( "PlayerStartPosition()" );
  }
  std::string class_name() const { return "PlayerStartPosition"; }

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
      float new_x = json_data["value"].value( "x", 0.0f );
      float new_y = json_data["value"].value( "y", 0.0f );

      // Set the base sf::Vector2f coordinates if they are positive
      // if json has 0,0 set, then use default component values
      // after first save to proper values, use the loaded values
      if ( new_x > 0 && new_y > 0 )
      {
        x = new_x;
        y = new_y;
      }
    }
  }

  virtual nlohmann::json serialize() const
  {
    nlohmann::json json_data;
    json_data["type"] = "sf::Vector2f";
    json_data["value"]["x"] = x;
    json_data["value"]["y"] = y;
    return json_data;
  }
};

} // namespace ProceduralMaze::Cmp::Persistent

#endif // __COMPONENTS_POSITION_HPP__