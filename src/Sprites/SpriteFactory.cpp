#include <SpriteFactory.hpp>

namespace ProceduralMaze::Sprites {

SpriteFactory::SpriteFactory()
    : m_sprite_metadata_list{
          { SpriteFactory::SpriteMetaType::WALL, "WALL", 1.f,
            MultiSprite{ "res/textures/walls_and_doors.png", { 0, 1, 2, 3, 4, 5, 6 } } },
          { SpriteFactory::SpriteMetaType::ROCK, "ROCK", 40.f,
            MultiSprite{ "res/textures/objects.png", { 147, 148 } } },
          { SpriteFactory::SpriteMetaType::POT, "POT", 1.f,
            MultiSprite{ "res/textures/objects.png", { 337, 339, 341 } } },
          { SpriteFactory::SpriteMetaType::BONES, "BONES", 1.f,
            MultiSprite{ "res/textures/objects.png", { 270, 271 } } },
          { SpriteFactory::SpriteMetaType::NPC, "NPC", 1.f,
            MultiSprite{ "res/textures/enemies-skeleton1_movement.png", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } } },
          { SpriteFactory::SpriteMetaType::DETONATED, "DETONATED", 1.f,
            MultiSprite{ "res/textures/tilemap_packed.png", { 42 } } },
          { SpriteFactory::SpriteMetaType::PLAYER, "PLAYER", 1.f,
            MultiSprite{ "res/textures/conjurer1.png", { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } } },
          // MultiSprite{ "res/textures/tilemap_packed.png", { 84, 85, 86, 87, 88, 96, 97, 98, 99, 100 } } },
          { SpriteFactory::SpriteMetaType::BOMB, "BOMB", 1.f, MultiSprite{ "res/textures/bomb.png", { 0 } } },
          { SpriteFactory::SpriteMetaType::EXTRA_HEALTH, "EXTRA_HEALTH", 30.f,
            MultiSprite{ "res/textures/objects.png", { 32 } } },
          { SpriteFactory::SpriteMetaType::EXTRA_BOMBS, "EXTRA_BOMBS", 40.f,
            MultiSprite{ "res/textures/objects.png", { 67 } } },
          { SpriteFactory::SpriteMetaType::INFINI_BOMBS, "INFINI_BOMBS", 1.f,
            MultiSprite{ "res/textures/objects.png", { 35 } } },
          { SpriteFactory::SpriteMetaType::CHAIN_BOMBS, "CHAIN_BOMBS", 20.f,
            MultiSprite{ "res/textures/objects.png", { 34 } } },
          { SpriteFactory::SpriteMetaType::LOWER_WATER, "LOWER_WATER", 40.f,
            MultiSprite{ "res/textures/objects.png", { 33 } } },
          { SpriteFactory::SpriteMetaType::EXPLOSION, "EXPLOSION", 1.f,
            MultiSprite{ "res/textures/fire_effect_and_bullet_16x16.png", { 66, 67, 68, 69, 70, 71 } } },
          { SpriteFactory::SpriteMetaType::FOOTSTEPS, "FOOTSTEPS", 1.f,
            MultiSprite{ "res/textures/footsteps.png", { 0 } } },
          { SpriteFactory::SpriteMetaType::SINKHOLE, "SINKHOLE", 1.f,
            MultiSprite{ "res/textures/sinkhole.png", { 0 } } },
          { SpriteFactory::SpriteMetaType::CORRUPTION, "CORRUPTION", 1.f,
            MultiSprite{ "res/textures/corruption.png", { 0 } } },
          { SpriteFactory::SpriteMetaType::WORMHOLE, "WORMHOLE", 1.f,
            MultiSprite{ "res/textures/objects.png", { 391, 392, 393, 423, 424, 425, 455, 456, 457 } } } }
{
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
  if ( auto data = get_random_spritedata( type_list, weights ) )
  {
    Cmp::Random random_picker( 0, data->m_multisprite.get_sprite_count() - 1 );
    return { data->type, random_picker.gen() };
  }
  return { SpriteMetaType::ROCK, 0 }; // Fallback
}

std::optional<SpriteFactory::SpriteMetaData> SpriteFactory::get_spritedata_by_type( SpriteMetaType type ) const
{
  auto it = std::find_if( m_sprite_metadata_list.begin(), m_sprite_metadata_list.end(),
                          [type]( const SpriteMetaData &meta ) { return meta.type == type; } );
  if ( it != m_sprite_metadata_list.end() ) { return *it; }
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

} // namespace ProceduralMaze::Sprites
