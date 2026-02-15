#include <Components/Persistent/PlayerStartPosition.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

namespace ProceduralMaze::Cmp::Persist
{

PlayerStartPosition::PlayerStartPosition( sf::Vector2f pos )
    : sf::Vector2f( pos )
{
}

std::string &PlayerStartPosition::get_value()
{
  std::stringstream ss;
  ss << "x: " << x << " y: " << y;
  static std::string str = ss.str();
  return str;
}

void PlayerStartPosition::deserialize( const nlohmann::json &json_data )
{
  if ( json_data.contains( "value" ) && json_data["value"].is_object() )
  {
    float new_x = json_data["value"].value( "x", 0.0f );
    float new_y = json_data["value"].value( "y", 0.0f );

    if ( new_x > 0 && new_y > 0 )
    {
      x = new_x;
      y = new_y;
    }
  }
}

nlohmann::json PlayerStartPosition::serialize() const
{
  nlohmann::json json_data;
  json_data["type"] = "sf::Vector2f";
  json_data["value"]["x"] = x;
  json_data["value"]["y"] = y;
  return json_data;
}

} // namespace ProceduralMaze::Cmp::Persist