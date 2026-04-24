#include <Sprites/SpriteMetaType.hpp>
#include <Systems/Render/UiData.hpp>

#include <charconv>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <source_location>

namespace ProceduralMaze::Render
{

UiData::UiData( const std::filesystem::path &map_file ) { deserialize( map_file ); }

nlohmann::json UiData::load_json_file( const std::filesystem::path &json_file )
{
  if ( not std::filesystem::exists( json_file ) )
  {
    SPDLOG_ERROR( "JSON file does not exist: {}", json_file.string() );
    throw std::runtime_error( "JSON file not found: " + json_file.string() );
  }

  std::ifstream fs( json_file );
  if ( not fs.is_open() )
  {
    SPDLOG_ERROR( "Unable to open JSON file: {}", json_file.string() );
    throw std::runtime_error( "Cannot open JSON file: " + json_file.string() );
  }

  nlohmann::json json;
  fs >> json;
  return json;
}

std::string UiData::get_string( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  if ( not json.contains( field ) )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " Missing JSON field: " + field );
  if ( not json.at( field ).is_string() )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " JSON field '" + field +
                              "' is not a string, got: " + std::string( json.at( field ).type_name() ) );
  return json.at( field ).get<std::string>();
}

float UiData::get_float( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  if ( not json.contains( field ) )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " Missing JSON field: " + field );
  if ( not json.at( field ).is_number() )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " JSON field '" + field +
                              "' is not a number, got: " + std::string( json.at( field ).type_name() ) );
  return json.at( field ).get<float>();
}

int UiData::get_int( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  if ( not json.contains( field ) )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " Missing JSON field: " + field );
  if ( not json.at( field ).is_number() )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " JSON field '" + field +
                              "' is not a number, got: " + std::string( json.at( field ).type_name() ) );
  return json.at( field ).get<int>();
}

sf::Color UiData::get_color( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  if ( not json.contains( field ) )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " Missing JSON field: " + field );
  if ( not json.at( field ).is_string() )
    throw std::runtime_error( std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " JSON field '" + field +
                              "' is not a string, got: " + std::string( json.at( field ).type_name() ) );

  auto hex_string = json.at( field ).get<std::string>();

  if ( not hex_string.empty() && hex_string[0] == '#' ) hex_string = hex_string.substr( 1 );

  uint32_t value{};
  const auto [ptr, ec] = std::from_chars( hex_string.data(), hex_string.data() + hex_string.size(), value, 16 );
  if ( ec != std::errc{} ) throw std::runtime_error( "Invalid hex color string: " + hex_string );

  const uint8_t a = ( value >> 24 ) & 0xFF;
  const uint8_t r = ( value >> 16 ) & 0xFF;
  const uint8_t g = ( value >> 8 ) & 0xFF;
  const uint8_t b = ( value >> 0 ) & 0xFF;
  return { r, g, b, a };
}

sf::FloatRect UiData::get_float_rect( const nlohmann::json &object )
{
  float w = get_float( object, "width" );
  float h = get_float( object, "height" );
  float x = get_float( object, "x" );
  float y = get_float( object, "y" );
  return { { x, y }, { w, h } };
}

std::string UiData::get_string_property( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  std::string loc_string = std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " ";
  if ( not json.contains( "properties" ) )
    throw std::runtime_error( loc_string + "Missing JSON property 'properties' in object " + get_string( json, "name" ) );
  for ( const auto &prop : json.at( "properties" ) )
  {
    if ( not prop.contains( "name" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'name' in object property" );
    if ( not prop.contains( "type" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'type' in object property" );
    if ( not prop.contains( "value" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'value' in object property" );
    if ( get_string( prop, "name" ) == field ) return get_string( prop, "value" );
  }
  throw std::runtime_error( loc_string + "Missing JSON property field in object: " + field );
}

int UiData::get_int_property( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  std::string loc_string = std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " ";
  if ( not json.contains( "properties" ) )
    throw std::runtime_error( loc_string + "Missing JSON property 'properties' in object " + get_string( json, "name" ) );
  for ( const auto &prop : json.at( "properties" ) )
  {
    if ( not prop.contains( "name" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'name' in object property" );
    if ( not prop.contains( "type" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'type' in object property" );
    if ( not prop.contains( "value" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'value' in object property" );
    if ( get_string( prop, "name" ) == field ) return get_int( prop, "value" );
  }
  throw std::runtime_error( loc_string + "Missing JSON property field in object: " + field );
}

sf::Color UiData::get_color_property( const nlohmann::json &json, const std::string &field, const std::source_location loc )
{
  std::string loc_string = std::string( loc.file_name() ) + ":" + std::to_string( loc.line() ) + " ";
  if ( not json.contains( "properties" ) )
    throw std::runtime_error( loc_string + "Missing JSON property 'properties' in object " + get_string( json, "name" ) );
  for ( const auto &prop : json.at( "properties" ) )
  {
    if ( not prop.contains( "name" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'name' in object property" );
    if ( not prop.contains( "type" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'type' in object property" );
    if ( not prop.contains( "value" ) ) throw std::runtime_error( loc_string + "Missing JSON property 'value' in object property" );
    if ( get_string( prop, "name" ) == field ) return get_color( prop, "value" );
  }
  throw std::runtime_error( loc_string + "Missing JSON property field in object: " + field );
}

void UiData::deserialize( const std::filesystem::path &scene_tiledata_path )
{

  nlohmann::json ui_tilemap_json = load_json_file( scene_tiledata_path );
  if ( not ui_tilemap_json.contains( "layers" ) ) return;
  for ( const auto &layer : ui_tilemap_json.at( "layers" ) )
  {
    SPDLOG_INFO( "Found Layers" );
    if ( not layer.contains( "objects" ) ) continue;
    for ( const auto &object : layer.at( "objects" ) )
    {

      if ( not object.contains( "name" ) ) throw std::runtime_error( "Missing JSON property 'name' in object" );
      if ( not object.contains( "type" ) ) throw std::runtime_error( "Missing JSON property 'type' in object: " + get_string( object, "name" ) );
      if ( not object.contains( "x" ) ) throw std::runtime_error( "Missing JSON property 'x' in object: " + get_string( object, "name" ) );
      if ( not object.contains( "y" ) ) throw std::runtime_error( "Missing JSON property 'y' in object: " + get_string( object, "name" ) );
      if ( not object.contains( "width" ) ) throw std::runtime_error( "Missing JSON property 'width' in object: " + get_string( object, "name" ) );
      if ( not object.contains( "height" ) ) throw std::runtime_error( "Missing JSON property 'height' in object: " + get_string( object, "name" ) );

      if ( get_string( object, "type" ) == "ui_outline" )
      {
        SPDLOG_INFO( "Found ui_outline: {}", get_string( object, "name" ) );
        m_outlines.emplace_back( get_float_rect( object ), get_string( object, "name" ), get_color_property( object, "fill_color" ),
                                 get_color_property( object, "line_color" ), get_int_property( object, "line_thickness" ) );
      }
      if ( get_string( object, "type" ) == "ui_label" )
      {
        SPDLOG_INFO( "Found ui_label: {}", get_string( object, "name" ) );
        m_labels.emplace_back( get_float_rect( object ), get_string( object, "name" ), get_int_property( object, "font_size" ),
                               get_string_property( object, "align" ) );
      }
      if ( get_string( object, "type" ) == "ui_meter" )
      {
        SPDLOG_INFO( "Found ui_meter: {}", get_string( object, "name" ) );
        m_meters.emplace_back( get_float_rect( object ), get_string( object, "name" ) );
      }
      if ( get_string( object, "type" ) == "ui_icon" )
      {
        m_icons.emplace_back( get_float_rect( object ), get_string( object, "name" ), get_string_property( object, "sprite_type" ),
                              get_int_property( object, "sprite_index" ), get_int_property( object, "sprite_scale" ) );
      }
    }
  }
}

} // namespace ProceduralMaze::Render