#include <Components/CryptObjectiveMultiBlock.hpp>
#include <Components/CryptObjectiveSegment.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/AltarMultiBlock.hpp>
#include <Components/AltarSegment.hpp>
#include <Components/Armable.hpp>
#include <Components/CryptEntrance.hpp>
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
#include <Systems/PersistSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

namespace ProceduralMaze::Sys::ProcGen
{

RandomLevelGenerator::RandomLevelGenerator( entt::registry &reg, sf::RenderWindow &window,
                                            Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank )
{
}

void RandomLevelGenerator::generate( RandomLevelGenerator::AreaShape shape, sf::Vector2u map_grid_size, bool gen_graves,
                                     bool gen_altars, bool gen_crypts )
{
  m_data.clear();
  switch ( shape )
  {
    case AreaShape::RECTANGLE:
      gen_rectangle_gamearea( map_grid_size );
      break;
    case AreaShape::CIRCLE:
      gen_circular_gamearea( map_grid_size );
      break;
    case AreaShape::CROSS:
      gen_cross_gamearea( map_grid_size );
      gen_crypt_main_objective( map_grid_size );
      break;
  }
  // gen_cross_gamearea( map_grid_size, 5, 0.5, 0.25, 20 );
  if ( gen_graves ) gen_grave_obstacles();
  if ( gen_altars ) gen_altar_obstacles();
  if ( gen_crypts ) gen_crypt_obstacles();
  gen_loot_containers( map_grid_size );
  gen_npc_containers( map_grid_size );
  gen_small_obstacles(); // these are post-processed by cellular automaton system
}

void RandomLevelGenerator::gen_rectangle_gamearea( sf::Vector2u map_grid_size )
{
  sf::Vector2f player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( getReg() );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 5.f,
                                            Cmp::RectBounds::ScaleCardinality::BOTH );

  unsigned int w = map_grid_size.x;
  unsigned int h = map_grid_size.y;

  for ( unsigned int x = 0; x < w; x++ )
  {
    for ( unsigned int y = 0; y < h; y++ )
    {
      sf::Vector2f new_pos( { x * Constants::kGridSquareSizePixelsF.x, y * Constants::kGridSquareSizePixelsF.y } );

      // condition for left, right, top, bottom borders
      bool isBorder = ( x == 0 ) || ( y == 0 ) || ( x == w - 1 ) || ( y == h - 1 );
      if ( isBorder ) { add_wall_entity( new_pos, 0 ); }
      else
      {
        // create world position entity, mark spawn area if in player start area
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        if ( pos_cmp.findIntersection( player_start_area.getBounds() ) )
        {
          Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f );
        }

        // track contiguous list of positions for proc gen
        m_data.push_back( entity );
      }
    }
  }
  SPDLOG_INFO( "Generated {} positions for random level.", m_data.size() );
}

void RandomLevelGenerator::gen_circular_gamearea( sf::Vector2u map_grid_size )
{
  auto player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( getReg() );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 5.f,
                                            Cmp::RectBounds::ScaleCardinality::BOTH );

  unsigned int w = map_grid_size.x;
  unsigned int h = map_grid_size.y;

  int cx = w / 2; // center in tiles
  int cy = h / 2;
  int r = std::min( w, h ) / 2; // radius in tiles

  int border = 2; // thickness in tiles

  int rOuter2 = r * r;
  int rInner = r - border;
  int rInner2 = rInner * rInner;

  for ( unsigned int x = 0; x < w; ++x )
  {
    for ( unsigned int y = 0; y < h; ++y )
    {
      int dx = static_cast<int>( x ) - cx;
      int dy = static_cast<int>( y ) - cy;
      int d2 = dx * dx + dy * dy;

      auto kGridSquareSizePixels = Constants::kGridSquareSizePixels;
      sf::Vector2f new_pos( x * kGridSquareSizePixels.x, y * kGridSquareSizePixels.y );

      if ( d2 <= rInner2 )
      {

        // create world position entity, mark spawn area if in player start area
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        if ( pos_cmp.findIntersection( player_start_area.getBounds() ) )
        {
          Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f );
        }

        // track the contiguous creation order entities so we can easily find its neighbours later
        m_data.push_back( entity );
      }
      else if ( d2 <= rOuter2 ) { add_wall_entity( new_pos, 0 ); }
      else
      {
        // outside circle (no tile)
      }
    }
  }
}

void RandomLevelGenerator::gen_cross_gamearea( sf::Vector2u map_grid_size, int vertArmHalfWidth, int horizArmHalfWidth,
                                               int horizOffset )
{
  auto player_start_pos = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::PlayerStartPosition>( getReg() );
  auto player_start_area = Cmp::RectBounds( player_start_pos, Constants::kGridSquareSizePixelsF, 3.f,
                                            Cmp::RectBounds::ScaleCardinality::BOTH );

  unsigned int w = map_grid_size.x; // in tiles
  unsigned int h = map_grid_size.y; // in tiles

  int cx = w / 2;
  int cy = h / 2;

  // arm half-length in tiles
  int vertHalfLength = h;
  int horizHalfLength = w;

  // helper: is (x,y) part of the cross?
  auto inCross = [&]( int x, int y ) -> bool
  {
    if ( x < 0 || y < 0 || x >= static_cast<int>( w ) || y >= static_cast<int>( h ) ) return false;

    int dx = x - cx;
    int dy = y - cy;

    // Vertical arm (centered on cx, cy)
    bool inVertical = std::abs( dx ) <= vertArmHalfWidth && std::abs( dy ) <= vertHalfLength;

    // Horizontal arm (centered on cy + horizOffset)
    bool inHorizontal = std::abs( ( dy - horizOffset ) ) <= horizArmHalfWidth && std::abs( dx ) <= horizHalfLength;

    return inVertical || inHorizontal;
  };

  for ( int x = 0; x < static_cast<int>( w ); ++x )
  {
    for ( int y = 0; y < static_cast<int>( h ); ++y )
    {

      auto kGridSquareSizePixels = Constants::kGridSquareSizePixels;
      sf::Vector2f new_pos( x * kGridSquareSizePixels.x, y * kGridSquareSizePixels.y );

      bool inside = inCross( x, y );
      if ( !inside ) continue; // outside cross, skip

      // 1-tile border: any 4-neighbor outside the cross
      bool isBorder = !inCross( x - 1, y ) || !inCross( x + 1, y ) || !inCross( x, y - 1 ) || !inCross( x, y + 1 );

      if ( isBorder ) { add_wall_entity( new_pos, 0 ); }
      else
      {
        // create world position entity, mark spawn area if in player start area
        auto entity = Factory::createWorldPosition( getReg(), new_pos );
        auto &pos_cmp = getReg().get<Cmp::Position>( entity );
        if ( pos_cmp.findIntersection( player_start_area.getBounds() ) )
        {
          Factory::addSpawnArea( getReg(), entity, new_pos.y - 16.0f );
        }

        // track the contiguous creation order entities so we can easily find its neighbours later
        m_data.push_back( entity );
      }
    }
  }
}

std::pair<entt::entity, Cmp::Position> RandomLevelGenerator::find_spawn_location( const Sprites::MultiSprite &ms,
                                                                                  unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        getReg(), Utils::Rnd::IncludePack<>{},
        Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::PlayableCharacter>{}, current_seed );

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

      for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] :
            getReg().view<Cmp::CryptObjectiveSegment, Cmp::Position>().each() )
      {
        if ( crypt_obj_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for reserved position collisions
      for ( auto [entity, reserved_cmp, reserved_pos_cmp] :
            getReg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
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
        SPDLOG_WARN( "Large Obstacle spawn: original seed {} was invalid, used seed {} instead (attempt {})", seed,
                     current_seed, attempts + 1 );
      }
      return { random_entity, random_pos };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})",
                kMaxAttempts, seed );
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
    Factory::createMultiblockSegments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( getReg(), random_entity,
                                                                                random_origin_position, ms );
  }
  else if ( ms.get_sprite_type().contains( "GRAVE" ) )
  {
    Factory::createMultiblock<Cmp::GraveMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( getReg(), random_entity,
                                                                                random_origin_position, ms );
  }
  else if ( ms.get_sprite_type().contains( "CRYPT" ) )
  {
    Factory::createMultiblock<Cmp::CryptMultiBlock>( getReg(), random_entity, random_origin_position, ms );
    Factory::createMultiblockSegments<Cmp::CryptMultiBlock, Cmp::CryptSegment>( getReg(), random_entity,
                                                                                random_origin_position, ms );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported multisprite type: {}", ms.get_sprite_type() );
    return;
  }
}

void RandomLevelGenerator::gen_grave_obstacles()
{
  auto grave_num_multiplier = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::GraveNumMultiplier>( getReg() );
  auto max_num_altars = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MaxNumAltars>( getReg() );

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
  auto max_num_altars = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MaxNumAltars>( getReg() );
  // ALTARS
  auto &altar_multisprite = m_sprite_factory.get_multisprite_by_type( "ALTAR.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    gen_large_obstacle( altar_multisprite, 0 );
  }
}

void RandomLevelGenerator::gen_crypt_obstacles()
{
  auto max_num_crypts = Sys::PersistSystem::get_persist_cmp<Cmp::Persist::MaxNumCrypts>( getReg() );
  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  auto &crypt_multisprite = m_sprite_factory.get_multisprite_by_type( "CRYPT.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    gen_large_obstacle( crypt_multisprite, 0 );
  }
}

void RandomLevelGenerator::gen_crypt_main_objective( sf::Vector2u map_grid_size )
{
  auto map_grid_sizef = sf::Vector2f( static_cast<float>( map_grid_size.x ) * Constants::kGridSquareSizePixelsF.x,
                                      static_cast<float>( map_grid_size.y ) * Constants::kGridSquareSizePixelsF.y );

  // target position for the objective: always center top of the map
  const auto &ms = m_sprite_factory.get_multisprite_by_type( "BAPHOMET" );
  Cmp::Position objective_position( { map_grid_sizef.x / 2.f, Constants::kGridSquareSizePixelsF.y * 2.f },
                                    ms.getSpriteSizePixels() );

  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, objective_position.position, objective_position.size );

  SPDLOG_INFO( "Placing main crypt objective at position ({}, {})", objective_position.position.x,
               objective_position.position.y );
  Factory::createMultiblock<Cmp::CryptObjectiveMultiBlock>( getReg(), entity, objective_position, ms );
  Factory::createMultiblockSegments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>(
      getReg(), entity, objective_position, ms );
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
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // pick a random loot container type and texture index
    // clang-format off
    auto [loot_type, rand_loot_tex_idx] = 
      m_sprite_factory.get_random_type_and_texture_index( { 
        "POT"
      } );
    // clang-format on

    float zorder = m_sprite_factory.get_sprite_size_by_type( loot_type ).y;

    Factory::createLootContainer( getReg(), random_entity, random_origin_position, loot_type, rand_loot_tex_idx,
                                  zorder );
  }
}

void RandomLevelGenerator::gen_npc_containers( sf::Vector2u map_grid_size )
{
  auto num_npc_containers = map_grid_size.x * map_grid_size.y / 200; // one NPC container per 200 grid squares

  for ( std::size_t i = 0; i < num_npc_containers; ++i )
  {
    auto [random_entity, random_origin_position] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayableCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

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

void RandomLevelGenerator::add_wall_entity( const sf::Vector2f &pos, std::size_t sprite_index )
{
  auto entity = getReg().create();
  getReg().emplace_or_replace<Cmp::Position>( entity, pos, Constants::kGridSquareSizePixelsF );
  getReg().emplace_or_replace<Cmp::Wall>( entity );
  getReg().emplace_or_replace<Cmp::SpriteAnimation>( entity, 0, 0, true, "WALL", sprite_index );
  getReg().emplace_or_replace<Cmp::ReservedPosition>( entity );
  getReg().emplace_or_replace<Cmp::ZOrderValue>( entity, pos.y );
  getReg().emplace_or_replace<Cmp::NoPathFinding>( entity );
}

} // namespace ProceduralMaze::Sys::ProcGen