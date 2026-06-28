#ifndef SRC_SYSTEMS_RENDER_UIDATA_HPP__
#define SRC_SYSTEMS_RENDER_UIDATA_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/JsonDeserializer.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Game::Render
{

class UiData : public Utils::JsonDeserializer
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
  void deserialize( const std::filesystem::path &scene_tiledata_path );

  std::vector<Outline> m_outlines;
  std::vector<Label> m_labels;
  std::vector<Text> m_texts;
  std::vector<Meter> m_meters;
  std::vector<Icon> m_icons;
};

} // namespace Game::Render

#endif // SRC_SYSTEMS_RENDER_UIDATA_HPP__
