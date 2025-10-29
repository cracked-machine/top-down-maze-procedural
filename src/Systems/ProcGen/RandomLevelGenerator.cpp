#include <BaseSystem.hpp>
#include <Destructable.hpp>
#include <Door.hpp>
#include <GraveSprite.hpp>
#include <LargeObstacle.hpp>
#include <LootContainer.hpp>
#include <NpcContainer.hpp>
#include <Persistent/MaxShrines.hpp>
#include <Persistent/PlayerStartPosition.hpp>
#include <PlayableCharacter.hpp>
#include <Position.hpp>
#include <ProcGen/RandomLevelGenerator.hpp>
#include <Random.hpp>
#include <RectBounds.hpp>
#include <ReservedPosition.hpp>
#include <SFML/System/Vector2.hpp>
#include <ShrineSprite.hpp>
#include <SpawnAreaSprite.hpp>
#include <SpriteFactory.hpp>
#include <Wall.hpp>

namespace ProceduralMaze::Sys::ProcGen {

RandomLevelGenerator::RandomLevelGenerator( ProceduralMaze::SharedEnttRegistry reg, sf::RenderWindow &window )
    : BaseSystem( reg, window )
{
  m_sprite_factory = get_persistent_component<std::shared_ptr<Sprites::SpriteFactory>>();
  gen_positions();
  gen_large_obstacles();
  gen_loot_containers();
  gen_npc_containers();
  gen_small_obstacles(); // these are post-processed by cellular automaton system
  gen_border();
  stats();
}

void RandomLevelGenerator::gen_positions()
{
  auto player_start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  auto player_start_area = Cmp::RectBounds( player_start_pos, kGridSquareSizePixelsF, 5.f,
                                            Cmp::RectBounds::ScaleCardinality::BOTH );

  for ( unsigned int x = 0; x < Sys::BaseSystem::kMapGridSize.x - kMapGridOffset.x; x++ )
  {
    for ( unsigned int y = 0; y < Sys::BaseSystem::kMapGridSize.y - kMapGridOffset.y; y++ )
    {
      auto entity = m_reg->create();
      sf::Vector2f new_pos( ( x + kMapGridOffset.x ) * Sys::BaseSystem::kGridSquareSizePixels.x,
                            ( y + kMapGridOffset.y ) * Sys::BaseSystem::kGridSquareSizePixels.y );

      m_reg->emplace<Cmp::Position>( entity, new_pos, kGridSquareSizePixelsF );
      auto &pos_cmp = m_reg->get<Cmp::Position>( entity );
      if ( pos_cmp.findIntersection( player_start_area.getBounds() ) )
      {
        // We need to reserve these positions for the player start area
        m_reg->emplace<Cmp::ReservedPosition>( entity );
        m_reg->emplace<Cmp::SpawnAreaSprite>( entity, "PLAYERSPAWN", 0, false );
      }

      // track the contiguous creation order of the entity so we can easily find its neighbours later
      m_data.push_back( entity );
    }
  }
}

void RandomLevelGenerator::gen_large_obstacle( std::optional<Sprites::MultiSprite> large_obstacle_sprite,
                                               Sprites::SpriteMetaType sprite_meta_type, unsigned long seed )
{
  // make sure the new large obstacle does not overlap with existing large obstacles
  auto [random_entity, random_origin_position] = get_random_position( {}, ExcludePack<Cmp::ReservedPosition>{}, seed );

  if ( random_entity != entt::null && large_obstacle_sprite.has_value() )
  {
    // place large obstacle - multiply the grid size to get pixel size!
    auto large_obst_grid_size = large_obstacle_sprite->get_grid_size();

    m_reg->emplace_or_replace<Cmp::LargeObstacle>( random_entity, sprite_meta_type, random_origin_position.position,
                                                   large_obst_grid_size.componentWiseMul( BaseSystem::kGridSquareSizePixels ) );

    SPDLOG_INFO( "Placed large obstacle ({}) at position ({}, {}). Grid size: {}x{}",
                 m_sprite_factory->get_spritedata_type_string( sprite_meta_type ), random_origin_position.position.x,
                 random_origin_position.position.y, large_obst_grid_size.width, large_obst_grid_size.height );

    // find any position-owning entities that intersect
    // with the new large obstacle and mark them as reserved
    auto new_large_obst_cmp = m_reg->get<Cmp::LargeObstacle>( random_entity );
    auto pos_view = m_reg->view<Cmp::Position>();
    for ( auto [entity, pos_cmp] : pos_view.each() )
    {
      if ( pos_cmp.findIntersection( new_large_obst_cmp ) )
      {
        // Calculate relative pixel positions within the large obstacle grid
        float rel_x = pos_cmp.position.x - random_origin_position.position.x;
        float rel_y = pos_cmp.position.y - random_origin_position.position.y;

        // Convert to grid coordinates
        int grid_x = static_cast<int>( rel_x / BaseSystem::kGridSquareSizePixels.x );
        int grid_y = static_cast<int>( rel_y / BaseSystem::kGridSquareSizePixels.y );

        // Calculate linear array index using relative grid distance from the origin grid position [0,0].
        // We can then use the index to look up the sprite and solid mask in the large obstacle sprite object
        // (method: row-major order: index = y * width + x)
        // E.g. for a 1x2 grid:
        //         [0]
        //         [1]
        // Top position: grid_y=0, grid_x=0 → sprite_index = 0 * 1 + 0 = 0
        // Bottom position: grid_y=1, grid_x=0 → sprite_index = 1 * 1 + 0 = 1
        // for a 4x2 grid:
        //         [0][1][2][3]
        //         [4][5][6][7]
        // Top-left position: grid_y=0, grid_x=0 → sprite_index = 0 * 4 + 0 = 0
        // Top-right position: grid_y=0, grid_x=3 → sprite_index = 0 * 4 + 3 = 3
        // Bottom-left position: grid_y=1, grid_x=0 → sprite_index = 1 * 4 + 0 = 4
        // Bottom-right position: grid_y=1, grid_x=3 → sprite_index = 1 * 4 + 3 = 7
        std::size_t calculated_grid_index = grid_y * large_obst_grid_size.width + grid_x;

        SPDLOG_DEBUG( "Calculated sprite index: {}", calculated_grid_index );

        SPDLOG_DEBUG( "Adding Cmp::ReservedPosition at ({}, {}) with sprite_index {}", pos_cmp.x, pos_cmp.y,
                      calculated_grid_index );

        // check multisprite solid_mask vector is at least as large as calculated index - default to true (solid) if out
        // of bounds
        bool new_solid_mask = true;
        auto solid_masks = large_obstacle_sprite->get_solid_mask();
        if ( !solid_masks.empty() && solid_masks.size() > calculated_grid_index )
        {
          new_solid_mask = solid_masks.at( calculated_grid_index );
        }

        if ( sprite_meta_type == "SHRINE" )
        {
          m_reg->emplace_or_replace<Cmp::ShrineSprite>( entity, sprite_meta_type, calculated_grid_index, new_solid_mask );
        }
        else if ( sprite_meta_type.contains( "GRAVE" ) )
        {
          m_reg->emplace_or_replace<Cmp::GraveSprite>( entity, sprite_meta_type, calculated_grid_index, new_solid_mask );
        }

        m_reg->emplace_or_replace<Cmp::ReservedPosition>( entity );
      }
    }
  }
}

void RandomLevelGenerator::gen_large_obstacles()
{
  auto max_num_shrines = get_persistent_component<Cmp::Persistent::MaxShrines>();
  // Get all available grave types dynamically from JSON
  auto grave_meta_types = m_sprite_factory->get_all_sprite_types_by_pattern( "GRAVE" );
  if ( grave_meta_types.empty() ) { SPDLOG_WARN( "No GRAVE multisprites found in SpriteFactory" ); }
  else
  {
    auto max_num_graves = max_num_shrines.get_value() * 10;
    for ( std::size_t i = 0; i < max_num_graves; ++i )
    {
      // Use the dynamically discovered grave types
      auto [sprite_metatype, unused_index] = m_sprite_factory->get_random_type_and_texture_index( grave_meta_types );
      auto multisprite = m_sprite_factory->get_multisprite_by_type( sprite_metatype );
      if ( !multisprite.has_value() )
      {
        SPDLOG_WARN( "No multisprite found for grave type {}", sprite_metatype );
        continue;
      }
      gen_large_obstacle( multisprite, sprite_metatype, 0 );
    }
  }
  auto shrine_multisprite = m_sprite_factory->get_multisprite_by_type( "SHRINE" );
  if ( !shrine_multisprite.has_value() ) { SPDLOG_WARN( "No SHRINE multisprite found in SpriteFactory" ); }
  else
  {
    for ( std::size_t i = 0; i < max_num_shrines.get_value(); ++i )
    {
      // Use the dynamically discovered shrine types
      gen_large_obstacle( shrine_multisprite, "SHRINE", 0 );
    }
  }
}

void RandomLevelGenerator::gen_small_obstacles()
{
  auto position_view = m_reg->view<Cmp::Position>( entt::exclude<Cmp::PlayableCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, pos] : position_view.each() )
  {
    // pick a random obstacle type and texture index
    // clang-format off
    auto [obst_type, rand_obst_tex_idx] = 
      m_sprite_factory->get_random_type_and_texture_index( { 
        "ROCK"
      } );
    // clang-format on

    m_reg->emplace<Cmp::Obstacle>( entity, obst_type, rand_obst_tex_idx, m_activation_selector.gen() );
    m_reg->emplace<Cmp::Destructable>( entity );
    m_reg->emplace<Cmp::Neighbours>( entity );
  }
}

void RandomLevelGenerator::gen_loot_containers()
{
  auto num_loot_containers = kMapGridSize.x * kMapGridSize.y / 60; // one loot container per 60 grid squares

  for ( std::size_t i = 0; i < num_loot_containers; ++i )
  {
    auto [random_entity, random_origin_position] = get_random_position(
        {}, ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [loot_type, rand_loot_tex_idx] = 
      m_sprite_factory->get_random_type_and_texture_index( { 
        "POT"
      } );
    // clang-format on

    m_reg->emplace_or_replace<Cmp::ReservedPosition>( random_entity );
    m_reg->emplace_or_replace<Cmp::Destructable>( random_entity );
    m_reg->emplace_or_replace<Cmp::LootContainer>( random_entity, loot_type, rand_loot_tex_idx );
  }
}

void RandomLevelGenerator::gen_npc_containers()
{
  auto num_npc_containers = kMapGridSize.x * kMapGridSize.y / 100; // one NPC container per 100 grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity, random_origin_position] = get_random_position(
        {}, ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [npc_type, rand_npc_tex_idx] =
      m_sprite_factory->get_random_type_and_texture_index( {
        "BONES"
      } );
    // clang-format on

    m_reg->emplace_or_replace<Cmp::ReservedPosition>( random_entity );
    m_reg->emplace_or_replace<Cmp::Destructable>( random_entity );
    m_reg->emplace_or_replace<Cmp::NpcContainer>( random_entity, npc_type, rand_npc_tex_idx );
  }
}

// These obstacles are for the map border.
// The textures are picked randomly, but their positions are fixed
void RandomLevelGenerator::gen_border()
{
  using namespace Sprites;
  const auto kGridSquareSizePixels = Sys::BaseSystem::kGridSquareSizePixels;
  const auto kMapGridSizePixels = kMapGridSize.componentWiseMul( kGridSquareSizePixels );
  std::size_t sprite_index = 0;

  // top and bottom edges
  for ( float x = 0; x < kMapGridSizePixels.x; x += kGridSquareSizePixels.x )
  {
    float top_edge_y_pos = ( kMapGridOffset.y - 1 ) * kGridSquareSizePixels.y;
    float bottom_edge_y_pos = ( kMapGridSizePixels.y );

    if ( x == 0 || x == kMapGridSizePixels.x - kGridSquareSizePixels.x )
      sprite_index = 0; // corner piece
    else
      sprite_index = 0; // horizontal piece

    add_wall_entity( { x, top_edge_y_pos }, sprite_index );
    add_wall_entity( { x, bottom_edge_y_pos }, sprite_index );
  }

  // left and right edges
  for ( float y = 0; y < kMapGridSizePixels.y; y += kGridSquareSizePixels.y )
  {
    float left_edge_x_pos = 0;
    float right_edge_x_pos = kMapGridSizePixels.x - kGridSquareSizePixels.x;

    if ( y == 0 || y == kMapGridSizePixels.y - 1 )
      sprite_index = 0; // corner piece
    else if ( y == ( kMapGridSizePixels.y / 2.f ) - kGridSquareSizePixels.y )
      sprite_index = 0; // door frame piece
    else if ( y == ( kMapGridSizePixels.y / 2.f ) + kGridSquareSizePixels.y )
      sprite_index = 0; // door frame piece
    else
      sprite_index = 0; // vertical piece

    // special case for door placement
    if ( y == ( kMapGridSizePixels.y / 2.f ) )
    {
      sprite_index = 0;
      // this entrance "door" is never open and for filtering simplicity we use the wall component
      add_wall_entity( { left_edge_x_pos, y }, sprite_index );
      // sprite_index = 1;
      // no more exit..it spawns in random position now
      add_wall_entity( { right_edge_x_pos, y }, sprite_index );
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
  m_reg->emplace<Cmp::Position>( entity, pos, kGridSquareSizePixelsF );
  m_reg->emplace<Cmp::Wall>( entity, "WALL", sprite_index );
}

void RandomLevelGenerator::add_door_entity( const sf::Vector2f &pos, std::size_t sprite_index, bool is_exit )
{
  auto entity = m_reg->create();
  m_reg->emplace<Cmp::Position>( entity, pos, kGridSquareSizePixelsF );
  m_reg->emplace<Cmp::Door>( entity, "WALL", sprite_index );
  if ( is_exit ) m_reg->emplace<Cmp::Exit>( entity );
}

void RandomLevelGenerator::stats()
{
  std::map<std::string, int> results;
  for ( auto [entity, _pos, _ob] : m_reg->view<Cmp::Position, Cmp::Obstacle>().each() )
  {
    results[m_sprite_factory->get_spritedata_type_string( _ob.m_type )]++;
  }
  for ( auto [entity, _pos, _lc] : m_reg->view<Cmp::Position, Cmp::LootContainer>().each() )
  {
    results[m_sprite_factory->get_spritedata_type_string( _lc.m_type )]++;
  }
  SPDLOG_INFO( "Object Pick distribution:" );
  for ( auto [bin, freq] : results )
  {
    SPDLOG_INFO( "[{}]:{}", bin, freq );
  }
}

} // namespace ProceduralMaze::Sys::ProcGen