#include <Components/Random.hpp>
#include <SFML/Graphics/Image.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>

#include <fstream>
#include <regex>
#include <spdlog/spdlog.h>
#include <string>

namespace nlohmann
{
//! @brief ADL hook used via nlohmann::basic_json::get (see SceneConfig::load below)
template <>
struct adl_serializer<ProceduralMaze::Sprites::MultiSprite>
{
  static void from_json( const json &j, ProceduralMaze::Sprites::MultiSprite &ms )
  {

    //! @brief lambda helper for error checking on JSON Single field types
    auto get_field = [&]<typename T>( const std::string &key, T &out )
    {
      if ( not j.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON sprites file" );
      try
      {
        out = j.at( key ).get<T>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing JSON sprites file: '" + key + "': " + e.what() );
      }
    };

    //! @brief lambda helper for error checking on JSON Single field types
    auto get_optional_list = [&]<typename T>( const std::string &key, std::vector<T> &out )
    {
      if ( not j.contains( key ) ) return;
      try
      {
        out = j.at( key ).get<std::vector<T>>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing JSON sprites file: '" + key + "': " + e.what() );
      }
    };

    //! @brief helper for error checking on JSON width/height vector types
    auto get_xy_field = [&]<typename TVec>( const std::string &key, TVec &out )
    {
      using TScalar = decltype( out.width );
      if ( !j.contains( key ) ) throw std::runtime_error( "Missing '" + key + "' from JSON sprites file" );
      try
      {
        out.width = j.at( key ).at( "width" ).get<TScalar>();
        out.height = j.at( key ).at( "height" ).get<TScalar>();
      } catch ( const nlohmann::json::type_error &e )
      {
        throw std::runtime_error( "Error parsing JSON sprites file: '" + key + "': " + e.what() );
      }
    };

    std::string display_name;
    get_field( "displayname", display_name );

    std::vector<float> zorder_list{};
    get_optional_list( "zorder", zorder_list );

    std::filesystem::path texture_path;
    get_field( "texture_path", texture_path );

    std::vector<uint32_t> sprite_indices;
    get_field( "sprite_indices", sprite_indices );

    unsigned int sprites_per_sequence;
    get_field( "sprites_per_sequence", sprites_per_sequence );

    unsigned int sprites_per_frame;
    get_field( "sprites_per_frame", sprites_per_frame );

    std::vector<bool> solid_mask{};
    get_optional_list( "solid_mask", solid_mask );

    ProceduralMaze::Sprites::SpriteSize grid_size;
    get_xy_field( "grid_size", grid_size );

    ms = ProceduralMaze::Sprites::MultiSprite{ "",        display_name,      zorder_list,          texture_path, sprite_indices,
                                               grid_size, sprites_per_frame, sprites_per_sequence, solid_mask };

    SPDLOG_INFO( "Loaded sprite metadata for type: {}, texture path: {}", "", texture_path.string() );
  }
};
} // namespace nlohmann

namespace ProceduralMaze::Sprites
{
void SpriteFactory::init()
{
  std::ifstream file( "res/json/sprite_metadata.json" );
  if ( !file.is_open() )
  {
    SPDLOG_CRITICAL( "Could not open sprite_metadata.json." );
    throw std::runtime_error( "Could not open sprite_metadata.json." );
  }

  nlohmann::json j;
  file >> j;

  if ( not j.contains( "sprites" ) ) throw std::runtime_error( "Missing 'sprites' from JSON scene config file" );
  const auto &sprites = j.at( "sprites" );
  for ( const auto &[ms_type, ms_object] : sprites.items() )
  {
    if ( not ms_object.contains( "multisprite" ) ) throw std::runtime_error( "Missing 'multisprite' from JSON scene config file" );
    const auto &multisprite = ms_object.at( "multisprite" );
    MultiSprite new_ms = multisprite.get<MultiSprite>();
    new_ms.set_sprite_type( ms_type );
    m_sprite_metadata_map[ms_type] = std::move( new_ms );
  }

  create_error_sprite();
}

void SpriteFactory::create_error_sprite()
{
  // Create procedural error texture (bright magenta/black checkerboard)
  sf::Image error_image( { 16, 16 }, sf::Color::Magenta );

  // Add black checkerboard pattern for visibility
  for ( unsigned int y = 0; y < 16; ++y )
  {
    for ( unsigned int x = 0; x < 16; ++x )
    {
      if ( ( x / 4 + y / 4 ) % 2 == 0 ) { error_image.setPixel( { x, y }, sf::Color::Black ); }
    }
  }

  // Create texture from image and save it
  [[maybe_unused]] bool result = m_error_texture.loadFromImage( error_image );

  // Create error sprite using the procedural texture
  m_error_metadata = MultiSprite{ "ERROR_SPRITE",  "Error Sprite", { 0.0 },
                                  m_error_texture, // Use the in-memory texture
                                  { 0 },           { 1, 1 },       1,       1, {} };
}

std::pair<SpriteMetaType, std::size_t> SpriteFactory::get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list )
{
  const MultiSprite &selected_data = get_random_spritedata( type_list );

  // Find the type that corresponds to this data
  for ( const auto &[type, metadata] : m_sprite_metadata_map )
  {
    if ( metadata.get_sprite_type() == selected_data.get_sprite_type() )
    {
      Cmp::RandomInt random_picker( 0, selected_data.get_sprite_count() - 1 );
      return { type, random_picker.gen() };
    }
  }
  // Fallback to error sprite if not found
  SPDLOG_ERROR( "Could not find matching sprite type in map, returning error sprite" );
  return { "ERROR_SPRITE", 0 };
}

SpriteMetaType SpriteFactory::get_random_type( std::vector<SpriteMetaType> type_list )
{
  const MultiSprite &selected_data = get_random_spritedata( type_list );

  return selected_data.get_sprite_type();
}

std::vector<SpriteMetaType> SpriteFactory::get_all_sprite_types_by_pattern( const std::string &pattern )
{
  std::vector<SpriteMetaType> types;

  try
  {
    // Try to use as regex first
    std::regex pattern_regex( pattern );

    for ( const auto &[type, _] : m_sprite_metadata_map )
    {
      if ( std::regex_search( type, pattern_regex ) ) { types.push_back( type ); }
    }
  } catch ( const std::regex_error &e )
  {
    // If regex fails, fallback to substring matching (current behavior)
    SPDLOG_DEBUG( "Pattern '{}' is not valid regex, using substring matching", pattern );
    for ( const auto &[type, _] : m_sprite_metadata_map )
    {
      if ( type.find( pattern ) != std::string::npos ) { types.push_back( type ); }
    }
  }

  return types;
}

const Sprites::MultiSprite &SpriteFactory::get_multisprite_by_type( const SpriteMetaType &type ) { return get_spritedata_by_type( type ); }

std::vector<SpriteMetaType> SpriteFactory::get_all_sprite_types()
{
  std::vector<SpriteMetaType> types;
  types.reserve( m_sprite_metadata_map.size() );

  for ( const auto &[type, _] : m_sprite_metadata_map )
  {
    types.push_back( type );
  }
  return types;
}

std::unordered_set<SpriteMetaType> SpriteFactory::get_all_sprite_types_set()
{
  std::unordered_set<SpriteMetaType> types;

  for ( const auto &pair : m_sprite_metadata_map )
  {
    types.insert( pair.first );
  }
  return types;
}

const MultiSprite &SpriteFactory::get_spritedata_by_type( const SpriteMetaType &type )
{
  auto it = m_sprite_metadata_map.find( type );
  if ( it != m_sprite_metadata_map.end() ) { return it->second; }
  return m_error_metadata;
}

const MultiSprite &SpriteFactory::get_random_spritedata( std::vector<SpriteMetaType> type_list )
{
  if ( type_list.empty() )
  {
    SPDLOG_ERROR( "Cannot get random sprite from empty type list, returning error sprite" );
    return m_error_metadata;
  }

  try
  {
    Cmp::RandomInt picker( 0.0f, type_list.size() - 1 );
    int pick = picker.gen();
    return get_spritedata_by_type( type_list[pick] );
  } catch ( ... )
  {
    // fallback
    return get_spritedata_by_type( type_list.back() );
  }
}

} // namespace ProceduralMaze::Sprites
