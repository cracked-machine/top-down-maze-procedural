#ifndef SRC_SYSTEM_RENDER_UIDATA_HPP_
#define SRC_SYSTEM_RENDER_UIDATA_HPP_

#include <Sprites/SpriteMetaType.hpp>
#include <nlohmann/json_fwd.hpp>
#include <source_location>

namespace ProceduralMaze::Render
{

class UiData
{
public:
  struct Outline
  {
    sf::FloatRect rect;
    std::string name;
    sf::Color fill_color;
    sf::Color line_color;
    int line_thickness;
  };

  struct Icon
  {
    sf::FloatRect rect;
    std::string name;
    Sprites::SpriteMetaType type;
    int index;
    int scale;
  };

  struct Label
  {
    sf::FloatRect rect;
    std::string name;
    int font_size;
    std::string align;
  };

  struct Meter
  {
    sf::FloatRect rect;
    std::string name;
  };

  struct Text
  {
    sf::FloatRect rect;
    std::string name;
    std::string value;
    int font_size;
  };

  UiData( const std::filesystem::path &map_file );
  nlohmann::json load_json_file( const std::filesystem::path &json_file );
  void deserialize( const std::filesystem::path &scene_tiledata_path );

  std::string get_string( const nlohmann::json &json_object, const std::string &field, std::source_location loc = std::source_location::current() );
  float get_float( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );
  int get_int( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );
  sf::Color get_color( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );

  int get_int_property( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );
  std::string get_string_property( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );
  sf::Color get_color_property( const nlohmann::json &json, const std::string &field, std::source_location loc = std::source_location::current() );

  sf::FloatRect get_float_rect( const nlohmann::json &json_object );

  std::vector<Outline> m_outlines;
  std::vector<Label> m_labels;
  std::vector<Text> m_texts;
  std::vector<Meter> m_meters;
  std::vector<Icon> m_icons;
};

} // namespace ProceduralMaze::Render

#endif // SRC_SYSTEM_RENDER_UIDATA_HPP_