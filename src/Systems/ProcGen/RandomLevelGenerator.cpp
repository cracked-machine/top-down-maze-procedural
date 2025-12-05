#include <Components/AbsoluteAlpha.hpp>
#include <Components/AltarMultiBlock.hpp>
#include <Components/CryptDoor.hpp>
#include <Components/CryptMultiBlock.hpp>
#include <Components/CryptSegment.hpp>
#include <Components/GraveMultiBlock.hpp>
#include <Components/NoPathFinding.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/SpriteAnimation.hpp>
#include <Components/ZOrderValue.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/GraveSegment.hpp>
#include <Components/LootContainer.hpp>
#include <Components/NpcContainer.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/PlayerStartPosition.hpp>
#include <Components/PlayableCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/Random.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/Wall.hpp>
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

void RandomLevelGenerator::generate()
{
  gen_positions();
  gen_border();
  gen_large_obstacles();
  gen_loot_containers();
  gen_npc_containers();
  gen_small_obstacles(); // these are post-processed by cellular automaton system
}

void RandomLevelGenerator::gen_positions()
{
  auto player_start_pos = get_persistent_component<Cmp::Persistent::PlayerStartPosition>();
  auto player_start_area = Cmp::RectBounds( player_start_pos, kGridSquareSizePixelsF, 5.f, Cmp::RectBounds::ScaleCardinality::BOTH );

  for ( unsigned int x = 0; x < Sys::BaseSystem::kMapGridSize.x - kMapGridOffset.x; x++ )
  {
    for ( unsigned int y = 0; y < Sys::BaseSystem::kMapGridSize.y - kMapGridOffset.y; y++ )
    {
      auto entity = getReg().create();
      sf::Vector2f new_pos( ( x + kMapGridOffset.x ) * Sys::BaseSystem::kGridSquareSizePixels.x,
                            ( y + kMapGridOffset.y ) * Sys::BaseSystem::kGridSquareSizePixels.y );

      getReg().emplace_or_replace<Cmp::Position>( entity, new_pos, kGridSquareSizePixelsF );
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

  // place large obstacle - multiply the grid size to get pixel size!
  auto large_obst_grid_size = ms.get_grid_size();

  // find any position-owning entities that intersect with the new large obstacle and mark them as reserved
  sf::FloatRect new_multiblock_bounds{};
  if ( ms.get_sprite_type().contains( "ALTAR" ) )
  {

    getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, ms.get_sprite_type(), 0 );
    getReg().emplace_or_replace<Cmp::AltarMultiBlock>( random_entity, random_origin_position.position,
                                                       large_obst_grid_size.componentWiseMul( BaseSystem::kGridSquareSizePixels ) );
    // clang-format off
    SPDLOG_INFO( "Placed AltarMultiBlock at position ({}, {}). Grid size: {}x{}", 
      random_origin_position.position.x,
      random_origin_position.position.y, 
      large_obst_grid_size.width, 
      large_obst_grid_size.height 
    );
    // clang-format on

    new_multiblock_bounds = getReg().get<Cmp::AltarMultiBlock>( random_entity );
  }
  else if ( ms.get_sprite_type().contains( "GRAVE" ) )
  {

    getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, ms.get_sprite_type(), 0 );
    getReg().emplace_or_replace<Cmp::GraveMultiBlock>( random_entity, random_origin_position.position,
                                                       large_obst_grid_size.componentWiseMul( BaseSystem::kGridSquareSizePixels ) );
    // clang-format off
    SPDLOG_DEBUG( "Placed GraveMultiBlock at position ({}, {}). Grid size: {}x{}", 
      random_origin_position.position.x,
      random_origin_position.position.y,
      large_obst_grid_size.width,
      large_obst_grid_size.height
    );
    // clang-format on

    new_multiblock_bounds = getReg().get<Cmp::GraveMultiBlock>( random_entity );
  }
  else if ( ms.get_sprite_type().contains( "CRYPT" ) )
  {
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, ms.get_sprite_type(), 0 );
    getReg().emplace_or_replace<Cmp::CryptMultiBlock>( random_entity, random_origin_position.position,
                                                       large_obst_grid_size.componentWiseMul( BaseSystem::kGridSquareSizePixels ) );
    // Set the z-order value so that the spawn area is rendered above ground but below everything else
    getReg().emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_origin_position.position.y + ms.getSpriteSizePixels().y );

    // clang-format off
    SPDLOG_INFO( "Placed CryptMultiBlock at position ({}, {}). Grid size: {}x{}", 
      random_origin_position.position.x,
      random_origin_position.position.y,
      large_obst_grid_size.width,
      large_obst_grid_size.height
    );
    // clang-format on

    new_multiblock_bounds = getReg().get<Cmp::CryptMultiBlock>( random_entity );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported multisprite type: {}", ms.get_sprite_type() );
    return;
  }

  auto pos_view = getReg().view<Cmp::Position>();
  for ( auto [entity, pos_cmp] : pos_view.each() )
  {
    if ( pos_cmp.findIntersection( new_multiblock_bounds ) )
    {
      // Calculate relative pixel positions within the large obstacle grid
      float rel_x = pos_cmp.position.x - random_origin_position.position.x;
      float rel_y = pos_cmp.position.y - random_origin_position.position.y;

      // Convert to relative grid coordinates
      int rel_grid_x = static_cast<int>( rel_x / BaseSystem::kGridSquareSizePixels.x );
      int rel_grid_y = static_cast<int>( rel_y / BaseSystem::kGridSquareSizePixels.y );

      // Calculate linear array index using relative grid distance from the origin grid position [0,0].
      // We can then use the index to look up the sprite and solid mask in the large obstacle sprite object
      // (method: row-major order: index = y * width + x)
      // Example for a 4x2 grid:
      //         [0][1][2][3]
      //         [4][5][6][7]
      // Top-left position: grid_y=0, grid_x=0 → sprite_index = 0 * 4 + 0 = 0
      // Top-right position: grid_y=0, grid_x=3 → sprite_index = 0 * 4 + 3 = 3
      // Bottom-left position: grid_y=1, grid_x=0 → sprite_index = 1 * 4 + 0 = 4
      // Bottom-right position: grid_y=1, grid_x=3 → sprite_index = 1 * 4 + 3 = 7
      std::size_t calculated_grid_index = rel_grid_y * large_obst_grid_size.width + rel_grid_x;
      SPDLOG_DEBUG( "Adding Cmp::ReservedPosition at ({}, {}) with sprite_index {}", pos_cmp.position.x, pos_cmp.position.y, calculated_grid_index );

      bool new_solid_mask = true;
      auto solid_masks = ms.get_solid_mask();
      if ( !solid_masks.empty() && solid_masks.size() > calculated_grid_index ) { new_solid_mask = solid_masks.at( calculated_grid_index ); }

      if ( ms.get_sprite_type().contains( "ALTAR" ) )
      {
        if ( new_solid_mask )
        {
          getReg().emplace_or_replace<Cmp::AltarSegment>( entity, new_solid_mask );
          getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
        }
        getReg().emplace_or_replace<Cmp::Armable>( entity );

        getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + ms.getSpriteSizePixels().y );
      }
      else if ( ms.get_sprite_type().contains( "GRAVE" ) )
      {
        if ( new_solid_mask )
        {
          getReg().emplace_or_replace<Cmp::GraveSegment>( entity, new_solid_mask );
          getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
        }
        getReg().emplace_or_replace<Cmp::Armable>( entity );
        getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + ms.getSpriteSizePixels().y );
      }
      else if ( ms.get_sprite_type().contains( "CRYPT" ) )
      {
        if ( new_solid_mask )
        {
          getReg().emplace_or_replace<Cmp::CryptSegment>( entity, new_solid_mask );
          getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
        }
        getReg().emplace_or_replace<Cmp::Armable>( entity );
        if ( calculated_grid_index == 10 ) // hardcoded for now - only the door segment
        {
          getReg().emplace_or_replace<Cmp::CryptDoor>( entity );
        }
      }
      getReg().emplace_or_replace<Cmp::ReservedPosition>( entity );
    }
  }
}

void RandomLevelGenerator::gen_large_obstacles()
{
  auto max_num_altars = get_persistent_component<Cmp::Persistent::MaxNumAltars>();
  auto grave_num_multiplier = get_persistent_component<Cmp::Persistent::GraveNumMultiplier>();

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

  // ALTARS
  auto &altar_multisprite = m_sprite_factory.get_multisprite_by_type( "ALTAR.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    gen_large_obstacle( altar_multisprite, 0 );
  }

  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  auto &crypt_multisprite = m_sprite_factory.get_multisprite_by_type( "CRYPT.closed" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
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
      getReg().emplace_or_replace<Cmp::Obstacle>( entity );
      // Set the z-order value so that the rock obstacles are rendered above everything else
      float zorder = m_sprite_factory.get_sprite_size_by_type( "ROCK" ).y;
      getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos_cmp.position.y + ( zorder * 2.f ) );
      getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
    }
    getReg().emplace_or_replace<Cmp::AbsoluteAlpha>( entity, 255 );
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, obst_type, rand_obst_tex_idx );
    getReg().emplace_or_replace<Cmp::Armable>( entity );
    getReg().emplace_or_replace<Cmp::Neighbours>( entity );
  }
}

void RandomLevelGenerator::gen_loot_containers()
{
  auto num_loot_containers = kMapGridSize.x * kMapGridSize.y / 60; // one loot container per 60 grid squares

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

    getReg().emplace_or_replace<Cmp::ReservedPosition>( random_entity );
    getReg().emplace_or_replace<Cmp::Armable>( random_entity );
    getReg().emplace_or_replace<Cmp::LootContainer>( random_entity );
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, loot_type, rand_loot_tex_idx );
    // Set the z-order value so that the spawn area is rendered above ground but below everything else
    float zorder = m_sprite_factory.get_sprite_size_by_type( "PLAYERSPAWN" ).y;
    getReg().emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_origin_position.position.y - zorder );
  }
}

void RandomLevelGenerator::gen_npc_containers()
{
  auto num_npc_containers = kMapGridSize.x * kMapGridSize.y / 200; // one NPC container per 200 grid squares

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

    getReg().emplace_or_replace<Cmp::ReservedPosition>( random_entity );
    getReg().emplace_or_replace<Cmp::Armable>( random_entity );
    getReg().emplace_or_replace<Cmp::NpcContainer>( random_entity );
    getReg().emplace_or_replace<Cmp::SpriteAnimation>( random_entity, 0, 0, true, npc_type, rand_npc_tex_idx );
    getReg().emplace_or_replace<Cmp::ZOrderValue>( random_entity, random_origin_position.position.y );
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