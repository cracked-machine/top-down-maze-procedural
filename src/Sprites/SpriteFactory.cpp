#include <SFML/Graphics/Image.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <fstream>
#include <string>

namespace ProceduralMaze::Sprites {

void SpriteFactory::init()
{
  std::ifstream file( "res/json/sprite_metadata.json" );
  if ( !file.is_open() )
  {
    SPDLOG_INFO( "Could not open sprite_metadata.json." );
    std::terminate();
  }
  nlohmann::json j;
  file >> j;

  // Parse JSON into sprite metadata map - no enum conversion needed!
  for ( const auto &[key, value] : j["sprites"].items() )
  {
    SpriteMetaData meta;
    meta.weight = value["weight"];

    std::filesystem::path texture_path = value["multisprite"]["texture_path"];
    std::vector<uint32_t> sprite_indices = value["multisprite"]["sprite_indices"].get<std::vector<uint32_t>>();
    SpriteSize grid_size = { value["multisprite"]["grid_size"]["width"], value["multisprite"]["grid_size"]["height"] };
    unsigned int sprites_per_frame = value["multisprite"]["sprites_per_frame"];
    unsigned int sprites_per_sequence = value["multisprite"]["sprites_per_sequence"];

    std::vector<bool> solid_mask{};
    if ( value["multisprite"].contains( "solid_mask" ) )
    {
      solid_mask = value["multisprite"]["solid_mask"].get<std::vector<bool>>();
    }

    meta.m_multisprite = MultiSprite{ key,       texture_path, sprite_indices, grid_size, sprites_per_frame, sprites_per_sequence,
                                      solid_mask };

    // Use the JSON key directly as the sprite type
    m_sprite_metadata_map[key] = std::move( meta );
  }

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
  m_error_metadata = SpriteMetaData{ 1.0f, MultiSprite{ "ERROR_SPRITE",
                                                        m_error_texture, // Use the in-memory texture
                                                        { 0 },
                                                        { 16, 16 },
                                                        1,
                                                        1,
                                                        {} } };
}

std::vector<SpriteMetaType> SpriteFactory::get_all_sprite_types() const
{
  std::vector<SpriteMetaType> types;
  types.reserve( m_sprite_metadata_map.size() );

  for ( const auto &[type, _] : m_sprite_metadata_map )
  {
    types.push_back( type );
  }
  return types;
}

bool SpriteFactory::has_sprite_type( const SpriteMetaType &type ) const
{
  return m_sprite_metadata_map.find( type ) != m_sprite_metadata_map.end();
}

std::vector<SpriteMetaType> SpriteFactory::get_all_sprite_types_by_pattern( const std::string &pattern ) const
{
  std::vector<SpriteMetaType> types;

  // Iterate directly over the map instead of creating a temporary vector
  for ( const auto &[type, _] : m_sprite_metadata_map )
  {
    if ( type.find( pattern ) != std::string::npos ) { types.push_back( type ); }
  }

  return types;
}

std::pair<SpriteMetaType, std::size_t> SpriteFactory::get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list,
                                                                                         std::vector<float> weights ) const
{
  const SpriteMetaData &selected_data = get_random_spritedata( type_list, weights );

  // Find the type that corresponds to this data
  for ( const auto &[type, metadata] : m_sprite_metadata_map )
  {
    if ( metadata.m_multisprite.get_sprite_type() == selected_data.m_multisprite.get_sprite_type() )
    {
      Cmp::RandomInt random_picker( 0, selected_data.m_multisprite.get_sprite_count() - 1 );
      return { type, random_picker.gen() };
    }
  }
  // Fallback to error sprite if not found
  SPDLOG_ERROR( "Could not find matching sprite type in map, returning error sprite" );
  return { "ERROR_SPRITE", 0 };
}

const SpriteFactory::SpriteMetaData &SpriteFactory::get_spritedata_by_type( const SpriteMetaType &type ) const
{
  auto it = m_sprite_metadata_map.find( type );
  if ( it != m_sprite_metadata_map.end() ) { return it->second; }
  return m_error_metadata;
}

const Sprites::MultiSprite &SpriteFactory::get_multisprite_by_type( const SpriteMetaType &type ) const
{
  return get_spritedata_by_type( type ).m_multisprite;
}

std::string SpriteFactory::get_spritedata_type_string( const SpriteMetaType &type ) const { return type; }

const SpriteFactory::SpriteMetaData &SpriteFactory::get_random_spritedata( std::vector<SpriteMetaType> type_list,
                                                                           std::vector<float> weights ) const
{
  if ( type_list.empty() )
  {
    SPDLOG_ERROR( "Cannot get random sprite from empty type list, returning error sprite" );
    return m_error_metadata;
  }

  // If weights aren't provided, use weights from metadata
  if ( weights.empty() )
  {
    weights.reserve( type_list.size() );
    for ( auto type : type_list )
    {
      const auto &meta = get_spritedata_by_type( type );
      weights.push_back( meta.weight );
    }
  }

  // Ensure weights and type_list have same size
  if ( weights.size() != type_list.size() ) { weights.resize( type_list.size(), 1.0f ); }

  float total_weight = std::accumulate( weights.begin(), weights.end(), 0.0f );

  // Generate random value between 0 and total weight
  Cmp::RandomFloat random_float( 0.0f, total_weight );
  float random_val = random_float.gen();

  // Select based on weights
  float cumulative_weight = 0.0f;
  for ( size_t i = 0; i < type_list.size(); ++i )
  {
    cumulative_weight += weights[i];
    if ( random_val <= cumulative_weight ) { return get_spritedata_by_type( type_list[i] ); }
  }

  // Fallback (shouldn't reach here normally)
  return get_spritedata_by_type( type_list.back() );
}

} // namespace ProceduralMaze::Sprites
