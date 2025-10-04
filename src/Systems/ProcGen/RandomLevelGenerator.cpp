#include <ProcGen/RandomLevelGenerator.hpp>

namespace ProceduralMaze::Sys::ProcGen {

void RandomLevelGenerator::gen_objects()
{
  for ( unsigned int x = 0; x < Sys::BaseSystem::kMapGridSize.x; x++ )
  {
    for ( unsigned int y = 0; y < Sys::BaseSystem::kMapGridSize.y; y++ )
    {
      auto entity = m_reg->create();
      m_reg->emplace<Cmp::Position>(
          entity, sf::Vector2f{
                      ( x * Sprites::MultiSprite::kDefaultSpriteDimensions.x ) +
                          ( Sys::BaseSystem::kMapGridOffset.x * Sprites::MultiSprite::kDefaultSpriteDimensions.x ),
                      ( y * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) +
                          ( Sys::BaseSystem::kMapGridOffset.y * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) } );
      // track the contiguous creation order of the entity so we can easily
      // find its neighbours later
      m_data.push_back( entity );
      auto sprite_factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
      if ( not sprite_factory )
      {
        SPDLOG_CRITICAL( "SpriteFactory not found in registry context" );
        std::get_terminate();
      }
      // pick a random obstacle type and texture index
      auto [obstacle_type, random_obstacle_texture_index] = sprite_factory->get_random_type_and_texture_index(
          { Sprites::SpriteFactory::SpriteMetaType::ROCK, Sprites::SpriteFactory::SpriteMetaType::POT,
            Sprites::SpriteFactory::SpriteMetaType::BONES } );
      m_reg->emplace<Cmp::Obstacle>( entity, obstacle_type, random_obstacle_texture_index, true,
                                     m_activation_selector.gen() );

      m_reg->emplace<Cmp::Neighbours>( entity );
    }
  }
}

// These obstacles are for the map border.
// The textures are picked randomly, but their positions are fixed
void RandomLevelGenerator::gen_border()
{
  std::size_t texture_index = 0;
  bool enabled = true;
  for ( float x = 0; x < kDisplaySize.x; x += Sprites::MultiSprite::kDefaultSpriteDimensions.x )
  {
    if ( x == 0 || x == kDisplaySize.x - Sprites::MultiSprite::kDefaultSpriteDimensions.x )
      texture_index = 2;
    else
      texture_index = 1;
    // top edge
    add_border_entity( { x, ( kMapGridOffset.y - 1 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.y },
                       texture_index );
    // bottom edge
    add_border_entity(
        { x, kMapGridOffset.y + ( ( kMapGridSize.y + 1 ) * Sprites::MultiSprite::kDefaultSpriteDimensions.y ) - 1 },
        texture_index );
  }
  for ( float y = 0; y < kDisplaySize.y; y += Sprites::MultiSprite::kDefaultSpriteDimensions.y )
  {
    if ( y == 0 || y == kDisplaySize.y - 1 )
      texture_index = 2;
    else if ( y == ( kDisplaySize.y / 2.f ) - Sprites::MultiSprite::kDefaultSpriteDimensions.y )
      texture_index = 3;
    else if ( y == ( kDisplaySize.y / 2.f ) )
      texture_index = 5; // closed door entrance
    else if ( y == ( kDisplaySize.y / 2.f ) + Sprites::MultiSprite::kDefaultSpriteDimensions.y )
      texture_index = 4;
    else
      texture_index = 0;
    // left edge
    add_border_entity( { 0, y }, texture_index );
    if ( y == ( kDisplaySize.y / 2.f ) )
    {
      texture_index = 6;
      enabled = false;
    } // open door exit
    // right edge
    add_border_entity( { static_cast<float>( kDisplaySize.x ) - Sprites::MultiSprite::kDefaultSpriteDimensions.x, y },
                       texture_index, enabled );
    enabled = true;
  }
}

void RandomLevelGenerator::add_border_entity( const sf::Vector2f &pos, std::size_t texture_index, bool enabled )
{
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( entity, pos );
  m_reg->emplace<Cmp::Obstacle>( entity, Sprites::SpriteFactory::SpriteMetaType::WALL, texture_index, true, enabled );
  m_reg->emplace<Cmp::Exit>( entity );
}

void RandomLevelGenerator::stats()
{
  std::map<std::string, int> results;
  for ( auto [entity, _pos, _ob] : m_reg->view<Cmp::Position, Cmp::Obstacle>().each() )
  {
    auto sprite_factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
    if ( not sprite_factory ) continue;
    results[sprite_factory->get_spritedata_type_string( _ob.m_type )]++;
  }
  SPDLOG_INFO( "Obstacle Pick distribution:" );
  for ( auto [bin, freq] : results )
  {
    SPDLOG_INFO( "[{}]:{}", bin, freq );
  }
}

} // namespace ProceduralMaze::Sys::ProcGen