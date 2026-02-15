#ifndef __COMPONENTS_PLAYERSTARTPOSITION_HPP__
#define __COMPONENTS_PLAYERSTARTPOSITION_HPP__

#include <SFML/System/Vector2.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace ProceduralMaze::Cmp::Persist
{

class PlayerStartPosition : public sf::Vector2f
{
public:
  PlayerStartPosition( sf::Vector2f pos = { 0.0f, 0.0f } );

  std::string class_name() const { return "PlayerStartPosition"; }
  std::string &get_value();
  void deserialize( const nlohmann::json &json_data );
  nlohmann::json serialize() const;
  const std::string get_detail() const { return ""; }
};

} // namespace ProceduralMaze::Cmp::Persist

#endif // __COMPONENTS_PLAYERSTARTPOSITION_HPP__