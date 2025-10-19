#include <SpriteFactory.hpp>

#include <fstream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace ProceduralMaze::Sprites {

SpriteFactory::SpriteFactory()
{
  std::ifstream file( "res/json/sprite_metadata.json" );
  if ( !file.is_open() )
  {
    SPDLOG_INFO( "Could not open sprite_metadata.json." );
    std::terminate();
  }
  nlohmann::json j;
  file >> j;

  // Parse JSON into sprite metadata map
  for ( const auto &[key, value] : j["sprites"].items() )
  {
    SpriteMetaType type = string_to_sprite_type( key );

    SpriteMetaData meta;
    meta.name = value["name"];
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

    meta.m_multisprite = MultiSprite{ texture_path,      sprite_indices,       grid_size,
                                      sprites_per_frame, sprites_per_sequence, solid_mask };

    m_sprite_metadata_map[type] = meta;
  }
}

std::optional<SpriteFactory::SpriteMetaData>
SpriteFactory::get_random_spritedata( std::vector<SpriteFactory::SpriteMetaType> type_list,
                                      std::vector<float> weights ) const
{
  if ( type_list.empty() )
  {
    SPDLOG_WARN( "Type list is empty" );
    return std::nullopt;
  }

  // If weights aren't provided, use weights from metadata
  if ( weights.empty() )
  {
    weights.reserve( type_list.size() );
    for ( auto type : type_list )
    {
      auto meta = get_spritedata_by_type( type );
      if ( meta ) { weights.push_back( meta->weight ); }
      else
      {
        weights.push_back( 1.0f ); // Default weight
      }
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

std::pair<SpriteFactory::SpriteMetaType, std::size_t>
SpriteFactory::get_random_type_and_texture_index( std::vector<SpriteMetaType> type_list,
                                                  std::vector<float> weights ) const
{
  auto selected_data = get_random_spritedata( type_list, weights );
  if ( selected_data )
  {
    // Find the type that corresponds to this data
    for ( const auto &[type, metadata] : m_sprite_metadata_map )
    {
      if ( metadata.name == selected_data->name )
      {
        Cmp::Random random_picker( 0, selected_data->m_multisprite.get_sprite_count() - 1 );
        return { type, random_picker.gen() };
      }
    }
  }
  return { SpriteMetaType::ROCK, 0 }; // Fallback
}

std::optional<SpriteFactory::SpriteMetaData> SpriteFactory::get_spritedata_by_type( SpriteMetaType type ) const
{
  auto it = m_sprite_metadata_map.find( type );
  if ( it != m_sprite_metadata_map.end() ) { return it->second; }
  return std::nullopt;
}

std::optional<Sprites::MultiSprite> SpriteFactory::get_multisprite_by_type( SpriteMetaType type ) const
{
  if ( auto data = get_spritedata_by_type( type ) ) { return data->m_multisprite; }
  return std::nullopt;
}

std::string SpriteFactory::get_spritedata_type_string( SpriteFactory::SpriteMetaType type ) const
{
  if ( auto data = get_spritedata_by_type( type ) ) { return data->name; }
  return "NOTFOUND";
}

SpriteFactory::SpriteMetaType SpriteFactory::string_to_sprite_type( const std::string &str ) const
{
  static const std::unordered_map<std::string, SpriteFactory::SpriteMetaType> map = {
      { "WALL", SpriteFactory::SpriteMetaType::WALL },
      { "ROCK", SpriteFactory::SpriteMetaType::ROCK },
      { "POT", SpriteFactory::SpriteMetaType::POT },
      { "BONES", SpriteFactory::SpriteMetaType::BONES },
      { "NPC", SpriteFactory::SpriteMetaType::NPC },
      { "DETONATED", SpriteFactory::SpriteMetaType::DETONATED },
      { "PLAYER", SpriteFactory::SpriteMetaType::PLAYER },
      { "BOMB", SpriteFactory::SpriteMetaType::BOMB },
      { "EXTRA_HEALTH", SpriteFactory::SpriteMetaType::EXTRA_HEALTH },
      { "EXTRA_BOMBS", SpriteFactory::SpriteMetaType::EXTRA_BOMBS },
      { "INFINI_BOMBS", SpriteFactory::SpriteMetaType::INFINI_BOMBS },
      { "CHAIN_BOMBS", SpriteFactory::SpriteMetaType::CHAIN_BOMBS },
      { "LOWER_WATER", SpriteFactory::SpriteMetaType::LOWER_WATER },
      { "EXPLOSION", SpriteFactory::SpriteMetaType::EXPLOSION },
      { "FOOTSTEPS", SpriteFactory::SpriteMetaType::FOOTSTEPS },
      { "SINKHOLE", SpriteFactory::SpriteMetaType::SINKHOLE },
      { "CORRUPTION", SpriteFactory::SpriteMetaType::CORRUPTION },
      { "WORMHOLE", SpriteFactory::SpriteMetaType::WORMHOLE },
      { "PILLAR", SpriteFactory::SpriteMetaType::PILLAR },
      { "PLINTH", SpriteFactory::SpriteMetaType::PLINTH } };

  auto it = map.find( str );
  if ( it != map.end() ) { return it->second; }
  throw std::invalid_argument( "Unknown sprite type: " + str );
}

} // namespace ProceduralMaze::Sprites
