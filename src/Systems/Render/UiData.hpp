#ifndef SRC_SYSTEMS_RENDER_UIDATA_HPP__
#define SRC_SYSTEMS_RENDER_UIDATA_HPP__

#include <Sprites/SpriteMetaType.hpp>
#include <Utils/JsonDeserializer.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Game::Render
{

//! @brief Loads and holds UI layout data (outlines, labels, texts, meters and icons) deserialized from a Tiled JSON UI map file.
class UiData : public Utils::JsonDeserializer
{
public:
  //! @brief A rectangular outline element in the UI layout (e.g. a panel border).
  struct Outline
  {
    sf::FloatRect rect;
    std::string name;
    sf::Color fill_color;
    sf::Color line_color;
    int line_thickness;
  };

  //! @brief A sprite icon element in the UI layout.
  struct Icon
  {
    sf::FloatRect rect;
    std::string name;
    Sprites::SpriteMetaType type;
    int index;
    int scale;
  };

  //! @brief A dynamic text label element in the UI layout (e.g. a stat value driven by game state).
  struct Label
  {
    sf::FloatRect rect;
    std::string name;
    int font_size;
    std::string align;
  };

  //! @brief A bar/meter element in the UI layout (e.g. health, fear).
  struct Meter
  {
    sf::FloatRect rect;
    std::string name;
  };

  //! @brief A static text element in the UI layout.
  struct Text
  {
    sf::FloatRect rect;
    std::string name;
    std::string value;
    int font_size;
  };

  //! @brief Construct a new UiData object and deserialize the given UI map file.
  //! @param map_file
  UiData( const std::filesystem::path &map_file );

  //! @brief Parse the Tiled JSON UI map file at `scene_tiledata_path` and populate the outline, label, text, meter and icon lists.
  //! @param scene_tiledata_path
  void deserialize( const std::filesystem::path &scene_tiledata_path );

  std::vector<Outline> m_outlines;
  std::vector<Label> m_labels;
  std::vector<Text> m_texts;
  std::vector<Meter> m_meters;
  std::vector<Icon> m_icons;
};

} // namespace Game::Render

#endif // SRC_SYSTEMS_RENDER_UIDATA_HPP__
