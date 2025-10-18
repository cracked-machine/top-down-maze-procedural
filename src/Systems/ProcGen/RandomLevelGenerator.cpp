#include <Door.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
#include <Wall.hpp>

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
      // track the contiguous creation order of the entity so we can easily find its neighbours later
      m_data.push_back( entity );
      auto sprite_factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
      // pick a random obstacle type and texture index
      auto [obstacle_type, random_obstacle_texture_index] = sprite_factory->get_random_type_and_texture_index(
          { Sprites::SpriteFactory::SpriteMetaType::ROCK, Sprites::SpriteFactory::SpriteMetaType::POT,
            Sprites::SpriteFactory::SpriteMetaType::BONES } );
      m_reg->emplace<Cmp::Obstacle>( entity, obstacle_type, random_obstacle_texture_index,
                                     m_activation_selector.gen() );

      m_reg->emplace<Cmp::Neighbours>( entity );
    }
  }
}

// These obstacles are for the map border.
// The textures are picked randomly, but their positions are fixed
void RandomLevelGenerator::gen_border()
{
  const auto kDefaultSpriteDimensions = Sprites::MultiSprite::kDefaultSpriteDimensions;
  std::size_t sprite_index = 0;

  // top and bottom edges
  for ( float x = 0; x < kDisplaySize.x; x += kDefaultSpriteDimensions.x )
  {
    float top_edge_y_pos = ( kMapGridOffset.y - 1 ) * kDefaultSpriteDimensions.y;
    float bottom_edge_y_pos = kMapGridOffset.y + ( ( kMapGridSize.y + 1 ) * kDefaultSpriteDimensions.y ) - 1;

    if ( x == 0 || x == kDisplaySize.x - kDefaultSpriteDimensions.x )
      sprite_index = 0; // corner piece
    else
      sprite_index = 0; // horizontal piece

    add_wall_entity( { x, top_edge_y_pos }, sprite_index );
    add_wall_entity( { x, bottom_edge_y_pos }, sprite_index );
  }

  // left and right edges
  for ( float y = 0; y < kDisplaySize.y; y += kDefaultSpriteDimensions.y )
  {
    float left_edge_x_pos = 0;
    float right_edge_x_pos = kDisplaySize.x - kDefaultSpriteDimensions.x;

    if ( y == 0 || y == kDisplaySize.y - 1 )
      sprite_index = 0; // corner piece
    else if ( y == ( kDisplaySize.y / 2.f ) - kDefaultSpriteDimensions.y )
      sprite_index = 0; // door frame piece
    else if ( y == ( kDisplaySize.y / 2.f ) + kDefaultSpriteDimensions.y )
      sprite_index = 0; // door frame piece
    else
      sprite_index = 0; // vertical piece

    // special case for door placement
    if ( y == ( kDisplaySize.y / 2.f ) )
    {
      sprite_index = 1;
      // this entrance "door" is never open and for filtering simplicity we use the wall component
      add_door_entity( { left_edge_x_pos, y }, sprite_index, false );
      sprite_index = 1;
      // this is the exit, we can filter on Cmp::Door for simple collsion detection
      add_door_entity( { right_edge_x_pos, y }, sprite_index, true );
    }
    else
    {
      add_wall_entity( { left_edge_x_pos, y }, sprite_index );
      add_wall_entity( { right_edge_x_pos, y }, sprite_index );
    }
  }
}

void RandomLevelGenerator::add_wall_entity( const sf::Vector2f &pos, std::size_t sprite_index )
{
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( entity, pos );
  m_reg->emplace<Cmp::Wall>( entity, Sprites::SpriteFactory::SpriteMetaType::WALL, sprite_index );
}

void RandomLevelGenerator::add_door_entity( const sf::Vector2f &pos, std::size_t sprite_index, bool is_exit )
{
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( entity, pos );
  m_reg->emplace<Cmp::Door>( entity, Sprites::SpriteFactory::SpriteMetaType::WALL, sprite_index );
  if ( is_exit ) m_reg->emplace<Cmp::Exit>( entity );
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