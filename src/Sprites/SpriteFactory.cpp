#include <SpriteFactory.hpp>

namespace ProceduralMaze::Sprites {

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
  if ( auto data = get_random_spritedata( type_list, weights ) )
  {
    Cmp::Random random_picker( 0, data->m_multisprite.get_sprite_count() - 1 );
    return { data->type, random_picker.gen() };
  }
  return { SpriteMetaType::ROCK, 0 }; // Fallback
}

std::optional<SpriteFactory::SpriteMetaData>
SpriteFactory::get_spritedata_by_type( SpriteMetaType type ) const
{
  auto it = std::find_if( m_sprite_metadata_list.begin(),
                          m_sprite_metadata_list.end(),
                          [type]( const SpriteMetaData &meta ) { return meta.type == type; } );
  if ( it != m_sprite_metadata_list.end() ) { return *it; }
  return std::nullopt;
}

std::optional<Sprites::MultiSprite>
SpriteFactory::get_multisprite_by_type( SpriteMetaType type ) const
{
  if ( auto data = get_spritedata_by_type( type ) ) { return data->m_multisprite; }
  return std::nullopt;
}

std::string SpriteFactory::get_spritedata_type_string( SpriteFactory::SpriteMetaType type ) const
{
  if ( auto data = get_spritedata_by_type( type ) ) { return data->name; }
  return "NOTFOUND";
}

} // namespace ProceduralMaze::Sprites
