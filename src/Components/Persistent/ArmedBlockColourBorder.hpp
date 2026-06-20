#ifndef SRC_CMPS_PERSIST_ARMEDBLOCKCOLOURBORDER_HPP_
#define SRC_CMPS_PERSIST_ARMEDBLOCKCOLOURBORDER_HPP_

#include <imgui.h>
#include <nlohmann/json.hpp>

namespace Game::Cmp::Persist
{

class ArmedBlockColourBorder : public sf::Color
{
public:
  ArmedBlockColourBorder() = default;
  ArmedBlockColourBorder( sf::Color rgb )
      : sf::Color( rgb )
  {
  }

  [[nodiscard]] std::string class_name() const { return "ArmedBlockColourBorder"; }
  [[nodiscard]] std::string get_detail() const { return ""; }

  void render_widget()
  {
    std::array<float, 4> colour = { static_cast<float>( r ) / 255.f, static_cast<float>( g ) / 255.f, static_cast<float>( b ) / 255.f,
                                    static_cast<float>( a ) / 255.f };
    if ( ImGui::ColorEdit4( "ArmedBlockColourBorder", colour.data() ) )
    {
      r = static_cast<uint8_t>( colour[0] * 255.f );
      g = static_cast<uint8_t>( colour[1] * 255.f );
      b = static_cast<uint8_t>( colour[2] * 255.f );
      a = static_cast<uint8_t>( colour[3] * 255.f );
    }
  }

  void deserialize( const nlohmann::json &json_data )
  {
    if ( json_data.contains( "value" ) and json_data["value"].is_object() )
    {
      r = json_data["value"].value( "r", 0u );
      g = json_data["value"].value( "g", 0u );
      b = json_data["value"].value( "b", 0u );
      a = json_data["value"].value( "a", 0u );
    }
  }

  [[nodiscard]] virtual nlohmann::json serialize() const
  {
    nlohmann::json json_data;
    json_data["type"] = "sf::Color";
    json_data["value"]["r"] = r;
    json_data["value"]["g"] = g;
    json_data["value"]["b"] = b;
    json_data["value"]["a"] = a;
    return json_data;
  }
};

} // namespace Game::Cmp::Persist

#endif // SRC_CMPS_PERSIST_ARMEDBLOCKCOLOURBORDER_HPP_