#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/CryptDoor.hpp>
#include <Components/CryptMultiBlock.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/GraveMultiBlock.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/Wall.hpp>
#include <Factory/LootFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Sprites/SpriteFactory.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>

namespace ProceduralMaze::Sys::ProcGen
{

RandomLevelGenerator::RandomLevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                            Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void RandomLevelGenerator::generate( sf::Vector2u map_grid_size, sf::Vector2f map_grid_offset, bool gen_graves, bool gen_altars, bool gen_crypts )
{
  m_data.clear();
  gen_positions( map_grid_size, map_grid_offset );
  gen_border( map_grid_size );
  if ( gen_graves ) gen_grave_obstacles();
  if ( gen_altars ) gen_altar_obstacles();
  if ( gen_crypts ) gen_crypt_obstacles();
  gen_loot_containers( map_grid_size );
  gen_npc_containers( map_grid_size );
  gen_small_obstacles(); // these are post-processed by cellular automaton system
}

void RandomLevelGenerator::gen_positions( sf::Vector2u map_grid_size, sf::Vector2f map_grid_offset )
{
  auto player_start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  auto player_start_area = Cmp::RectBounds( player_start_pos, kGridSquareSizePixelsF, 5.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  for ( unsigned int x = 0; x < map_grid_size.x - map_grid_offset.x; x++ )
  {
    for ( unsigned int y = 0; y < map_grid_size.y - map_grid_offset.y; y++ )
    {
      auto entity = getReg().create();
      sf::Vector2f new_pos( ( x + map_grid_offset.x ) * Sys::BaseSystem::kGridSquareSizePixels.x,
                            ( y + map_grid_offset.y ) * Sys::BaseSystem::kGridSquareSizePixels.y );

      getReg().emplace_or_replace<Cmp::Position>( entity, new_pos, kGridSquareSizePixelsF );
      getReg().emplace_or_replace<Cmp::Neighbours>( entity );
      auto &pos_cmp = getReg().get<Cmp::Position>( entity );
      if ( pos_cmp.findIntersection( player_start_area.getBounds() ) )
      {
        // We need to reserve these positions for the player start area
        getReg().emplace_or_replace<Cmp::ReservedPosition>( entity );
        getReg().emplace_or_replace<Cmp::SpawnArea>( entity, false );
        getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
        getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "PLAYERSPAWN", 0 );
        getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y - m_sprite_factory.get_sprite_size_by_type( "PLAYERSPAWN" ).y );
      }

      // track the contiguous creation order of the entity so we can easily find its neighbours later
      m_data.push_back( entity );
    }
  }
  SPDLOG_INFO( "Generated {} positions for random level.", m_data.size() );
}

std::pair<entt::entity, Cmp::Position> RandomLevelGenerator::find_spawn_location( const Sprites::MultiSprite &ms, unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = get_random_position( IncludePack<>{}, ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::PlayableCharacter>{},
                                                            current_seed );

    auto lo_sprite_size = m_sprite_factory.get_sprite_size_by_type( ms.get_sprite_type() );
    Cmp::RectBounds new_lo_hitbox( random_pos.position, lo_sprite_size, 1.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : getReg().view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : getReg().view<Cmp::GraveSegment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for altar collisions
      for ( auto [entity, altar_cmp, altar_pos_cmp] : getReg().view<Cmp::AltarSegment, Cmp::Position>().each() )
      {
        if ( altar_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for crypt collisions
      for ( auto [entity, crypt_cmp, crypt_pos_cmp] : getReg().view<Cmp::CryptSegment, Cmp::Position>().each() )
      {
        if ( crypt_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for reserved position collisions
      for ( auto [entity, reserved_cmp, reserved_pos_cmp] : getReg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
      {
        if ( reserved_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for spawn area collisions
      for ( auto [entity, spawn_cmp, spawn_pos_cmp] : getReg().view<Cmp::SpawnArea, Cmp::Position>().each() )
      {
        if ( spawn_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for playable character collisions
      for ( auto [entity, player_cmp, player_pos_cmp] : getReg().view<Cmp::PlayableCharacter, Cmp::Position>().each() )
      {
        if ( player_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      return true;
    };

    if ( is_valid() )
    {
      if ( current_seed != seed && seed > 0 )
      {
        SPDLOG_WARN( "Large Obstacle spawn: original seed {} was invalid, used seed {} instead (attempt {})", seed, current_seed, attempts + 1 );
      }
      return { random_entity, random_pos };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

void RandomLevelGenerator::gen_large_obstacle( const Sprites::MultiSprite &ms, unsigned long seed )
{
  auto [random_entity, random_origin_position] = find_spawn_location( ms, seed );
  if ( random_entity == entt::null )
  {
    SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
    return;
  }

  if ( ms.get_sprite_type().contains( "ALTAR" ) )
  {
    Factory::createMultiblock<Cmp::AltarMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type().contains( "GRAVE" ) )
  {
    Factory::createMultiblock<Cmp::GraveMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else if ( ms.get_sprite_type().contains( "CRYPT" ) )
  {
    Factory::createMultiblock<Cmp::CryptMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::CryptMultiBlock, Cmp::CryptSegment>( getReg(), random_entity, random_origin_position, ms );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported multisprite type: {}", ms.get_sprite_type() );
    return;
  }
}

void RandomLevelGenerator::gen_grave_obstacles()
{
  auto grave_num_multiplier = get_persistent_component<Cmp::Persistent::GraveNumMultiplier>();
  auto max_num_altars = get_persistent_component<Cmp::Persistent::MaxNumAltars>();

  // GRAVES
  auto grave_meta_types = m_sprite_factory.get_all_sprite_types_by_pattern( "^GRAVE\\d+\\.closed$" );
  if ( grave_meta_types.empty() ) { SPDLOG_WARN( "No GRAVE multisprites found in SpriteFactory" ); }
  else
  {
    uint8_t max_num_graves = max_num_altars.get_value() * grave_num_multiplier.get_value();
    for ( std::size_t i = 0; i < max_num_graves; ++i )
    {
      auto [sprite_metatype, unused_index] = m_sprite_factory.get_random_type_and_texture_index( grave_meta_types );
      auto &multisprite = m_sprite_factory.get_multisprite_by_type( sprite_metatype );
      gen_large_obstacle( multisprite, 0 );
    }
  }
}

void RandomLevelGenerator::gen_altar_obstacles()
{
  auto max_num_altars = get_persistent_component<Cmp::Persistent::MaxNumAltars>();
  // ALTARS
  auto &altar_multisprite = m_sprite_factory.get_multisprite_by_type( "ALTAR.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    gen_large_obstacle( altar_multisprite, 0 );
  }
}

void RandomLevelGenerator::gen_crypt_obstacles()
{
  auto max_num_crypts = get_persistent_component<Cmp::Persistent::MaxNumCrypts>();
  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  auto &crypt_multisprite = m_sprite_factory.get_multisprite_by_type( "CRYPT.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    gen_large_obstacle( crypt_multisprite, 0 );
  }
}

void RandomLevelGenerator::gen_small_obstacles()
{
  auto position_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::PlayableCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    // clang-format off
    auto [obst_type, rand_obst_tex_idx] = 
      m_sprite_factory.get_random_type_and_texture_index( { 
        "ROCK"
      } );
    // clang-format on

    if ( Cmp::RandomInt{ 0, 1 }.gen() == 1 )
    {
      float zorder = m_sprite_factory.get_sprite_size_by_type( "ROCK" ).y;
      // Set the z-order value so that the rock obstacles are rendered above everything else
      Factory::createObstacle( getReg(), entity, pos_cmp, obst_type, rand_obst_tex_idx, ( zorder * 2.f ) );
    }
  }
}

void RandomLevelGenerator::gen_loot_containers( sf::Vector2u map_grid_size )
{
  auto num_loot_containers = map_grid_size.x * map_grid_size.y / 60; // one loot container per 60 grid squares

  for ( std::size_t i = 0; i < num_loot_containers; ++i )
  {
    auto [random_entity,
          random_origin_position] = get_random_position( {}, ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [loot_type, rand_loot_tex_idx] = 
      m_sprite_factory.get_random_type_and_texture_index( { 
        "POT"
      } );
    // clang-format on

    float zorder = m_sprite_factory.get_sprite_size_by_type( loot_type ).y;

    Factory::createLootContainer( getReg(), random_entity, random_origin_position, loot_type, rand_loot_tex_idx, zorder );
  }
}

void RandomLevelGenerator::gen_npc_containers( sf::Vector2u map_grid_size )
{
  auto num_npc_containers = map_grid_size.x * map_grid_size.y / 200; // one NPC container per 200 grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity,
          random_origin_position] = get_random_position( {}, ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [npc_type, rand_npc_tex_idx] =
      m_sprite_factory.get_random_type_and_texture_index( {
        "BONES"
      } );
    // clang-format on

    Factory::createNpcContainer( getReg(), random_entity, random_origin_position, npc_type, rand_npc_tex_idx, 0.f );
  }
}

// These obstacles are for the map border.
// The textures are picked randomly, but their positions are fixed
void RandomLevelGenerator::gen_border( sf::Vector2u map_grid_size )
{
  using namespace Sprites;
  const auto kGridSquareSizePixels = Sys::BaseSystem::kGridSquareSizePixels;

  const auto kMapGridSizePixels = map_grid_size.componentWiseMul( kGridSquareSizePixels );
  std::size_t sprite_index = 0;

  // top and bottom edges
  for ( float x = 0; x < kMapGridSizePixels.x; x += kGridSquareSizePixels.x )
  {
    float top_edge_y_pos = ( kGraveyardMapGridOffset.y - 1 ) * kGridSquareSizePixels.y;
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
    float right_edge_x_pos = kMapGridSizePixels.x;

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
  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, pos, kGridSquareSizePixelsF );
  getReg().emplace_or_replace<Cmp::Wall>( entity );
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", sprite_index );
  getReg().emplace_or_replace<Cmp::ReservedPosition>( entity );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
}

} // namespace ProceduralMaze::Sys::ProcGen