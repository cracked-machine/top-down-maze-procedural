#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/HolyWell/HolyWellMultiBlock.hpp>
#include <Components/HolyWell/HolyWellSegment.hpp>
#include <Components/Inventory/CarryItem.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Ruin/RuinSegment.hpp>
#include <Constants.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <Ruin/RuinHexagramMultiBlock.hpp>
#include <Ruin/RuinHexagramSegment.hpp>
#include <Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Ruin/RuinStairsUpperMultiBlock.hpp>
#include <SFML/System/Vector2.hpp>

#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptMultiBlock.hpp>
#include <Components/Crypt/CryptSegment.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/Wall.hpp>

#include <Sprites/MultiSprite.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/ProcGen/RandomLevelGenerator.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>
#include <memory>

#include <PathFinding/SpatialHashGrid.hpp>
#include <ranges>
#include <spdlog/spdlog.h>

namespace ProceduralMaze::Sys::ProcGen
{

RandomLevelGenerator::RandomLevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory,
                                            Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_obstacle_sm( std::make_unique<PathFinding::SpatialHashGrid>() ),
      m_void_sm( std::make_unique<PathFinding::SpatialHashGrid>() )
{
}

PathFinding::SpatialHashGrid &RandomLevelGenerator::get_obstacle_sm() { return *m_obstacle_sm; }
PathFinding::SpatialHashGrid &RandomLevelGenerator::get_void_sm() { return *m_void_sm; }

void RandomLevelGenerator::gen_game_area( const Scene::SceneData &scene_map )
{
  auto [map_size_grid, map_size_pixel] = scene_map.map_size();
  auto w = map_size_grid.x;

  bool added_wall_already = false;
  [[maybe_unused]] const Sprites::MultiSprite &wall_ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_wall" );
  for ( const auto [i, tile] : std::views::enumerate( scene_map.wall_tilelayer() ) )
  {
    added_wall_already = true;
    int row = i / w; // increments every 'w' tiles
    int col = i % w; // wraps back to zero every 'w' tiles
    [[maybe_unused]] sf::Vector2f new_pos( { col * Constants::kGridSizePxF.x, row * Constants::kGridSizePxF.y } );
    if ( tile >= scene_map.wall_first_gid() ) { Factory::add_wall_entity( getReg(), new_pos, wall_ms, tile - scene_map.wall_first_gid() ); }
  }

  for ( const auto &solid : scene_map.solid_objectlayer() )
  {
    Factory::add_solid_player( getReg(), solid );
    Factory::add_solid_npc( getReg(), solid );
  }

  for ( const auto [i, tile] : std::views::enumerate( scene_map.levelgen_tilelayer() ) )
  {
    int col = i % w; // wraps back to zero every 'w' tiles
    int row = i / w; // increments every 'w' tiles
    sf::Vector2f new_pos( col * Constants::kGridSizePxF.x, row * Constants::kGridSizePxF.y );
    if ( tile == scene_map.void_tile_id() )
    {
      Cmp::Position new_pos_cmp( new_pos, Constants::kGridSizePxF );
      auto entt = Factory::create_void_pos( getReg(), new_pos_cmp );
      m_void_sm->insert( entt, new_pos_cmp );
    }
    else if ( not added_wall_already and tile == scene_map.wall_tile_id() )
    {
      const Sprites::MultiSprite &wall_ms = m_sprite_factory.get_multisprite_by_type( "CRYPT.interior_sb" );
      Factory::add_wall_entity( getReg(), new_pos, wall_ms, 0 );
    }
    else if ( tile == scene_map.open_tile_id() ) { Factory::create_world_pos( getReg(), new_pos ); }
    else if ( tile == scene_map.spawn_tile_id() )
    {
      auto entity = Factory::create_world_pos( getReg(), new_pos );
      Factory::add_spawn_area( getReg(), entity, new_pos.y - 16.0f );
    }
    else if ( tile == scene_map.exit_tile_id() ) { Factory::create_crypt_exit( getReg(), new_pos ); }
    else if ( tile == scene_map.reserved_tile_id() ) { Factory::add_reservedposition( getReg(), new_pos ); }
  }

  for ( const auto &[ms_type, pos] : scene_map.multiblock_objectlayer() )
  {
    const auto &ms = m_sprite_factory.get_multisprite_by_type( ms_type );
    if ( ms_type == "HOLYWELL.interior_well" )
    {
      Factory::add_multiblock_with_segments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( getReg(), pos, ms );
    }
    else if ( ms_type == "CRYPT.interior_objective_closed" )
    {
      Factory::add_multiblock_with_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( getReg(), pos, ms );
    }
    else if ( ms_type == "NPC.dr_knox" )
    {
      auto npc_entt = getReg().create();
      getReg().emplace_or_replace<Cmp::Position>( npc_entt, pos, Constants::kGridSizePxF );
      Factory::create_npc( m_reg, npc_entt, "NPC.dr_knox" );
    }
    else if ( ms_type == "RUIN.interior_staircase_going_up" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( getReg(), pos, ms );
    }
    else if ( ms_type == "RUIN.interior_staircase_going_down" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( getReg(), pos, ms );
    }
    else if ( ms_type == "RUIN.interior_staircase_upper_balustrade" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( getReg(), pos, ms );
    }
    else if ( ms_type == "RUIN.interior_hexagram3x3" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( m_reg, pos, ms );
    }
    else if ( ms_type.contains( "CARRYITEM" ) )
    {
      auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( getReg() );
      // prevent infinite respawns in the RuinSceneUpperFloor
      if ( inventory_type == "CARRYITEM.witchesjar" ) continue;

      // make sure we mark the *world* entt as reserved
      auto world_pos_entt = Utils::get_world_pos_entt( m_reg, Cmp::Position( pos, ms.getSpriteSizePixels() ) );
      if ( world_pos_entt != entt::null )
      {
        getReg().emplace_or_replace<Cmp::ReservedPosition>( world_pos_entt );
        Factory::create_carry_item( getReg(), Cmp::Position( pos, ms.getSpriteSizePixels() ), ms_type );
      }
    }
  }
}

void RandomLevelGenerator::gen_graveyard_exterior_obstacles()
{
  auto position_view = getReg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {

    if ( Cmp::RandomInt{ 0, 1 }.gen() == 1 )
    {
      const Sprites::MultiSprite &ms = m_sprite_factory.get_multisprite_by_type( "ROCK" );
      auto [_, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "ROCK" } );
      Factory::create_obstacle( getReg(), entity, pos_cmp, ms, rand_obst_tex_idx );
      m_obstacle_sm->insert( entity, pos_cmp );
    }
  }
}

void RandomLevelGenerator::gen_graveyard_exterior_multiblocks()
{
  auto grave_num_multiplier = Sys::PersistSystem::get<Cmp::Persist::GraveNumMultiplier>( getReg() );
  auto max_num_altars = Sys::PersistSystem::get<Cmp::Persist::MaxNumAltars>( getReg() );
  auto max_num_crypts = Sys::PersistSystem::get<Cmp::Persist::MaxNumCrypts>( getReg() );
  std::size_t max_number_holywells = 1;
  std::size_t max_number_ruins = 1;

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
      do_gen_graveyard_exterior_multiblock( multisprite, 0 );
    }
  }

  // ALTARS
  auto &altar_multisprite = m_sprite_factory.get_multisprite_by_type( "ALTAR.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( altar_multisprite, 0 );
  }

  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  auto &crypt_multisprite = m_sprite_factory.get_multisprite_by_type( "CRYPT.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( crypt_multisprite, 0 );
  }

  auto &holywell_multisprite = m_sprite_factory.get_multisprite_by_type( "HOLYWELL.exterior_building" );
  for ( std::size_t i = 0; i < max_number_holywells; ++i )
  {
    do_gen_graveyard_exterior_multiblock( holywell_multisprite, 0 );
  }

  auto &ruin_multisprite = m_sprite_factory.get_multisprite_by_type( "RUIN.exterior_building" );
  for ( std::size_t i = 0; i < max_number_ruins; ++i )
  {
    do_gen_graveyard_exterior_multiblock( ruin_multisprite, 0 );
  }
}

void RandomLevelGenerator::do_gen_graveyard_exterior_multiblock( const Sprites::MultiSprite &ms, unsigned long seed )
{
  auto [random_entity, random_origin_position] = find_spawn_location( ms, seed );
  if ( random_entity == entt::null )
  {
    SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
    return;
  }

  if ( ms.get_sprite_type().contains( "ALTAR" ) )
  {
    Factory::add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( getReg(), random_origin_position.position, ms );
  }
  else if ( ms.get_sprite_type().contains( "GRAVE" ) )
  {
    Factory::add_multiblock_with_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( getReg(), random_origin_position.position, ms );
  }
  else if ( ms.get_sprite_type() == "CRYPT.closed" )
  {
    Factory::add_multiblock_with_segments<Cmp::CryptMultiBlock, Cmp::CryptSegment>( getReg(), random_origin_position.position, ms );
  }
  else if ( ms.get_sprite_type() == "HOLYWELL.exterior_building" )
  {
    Factory::add_multiblock_with_segments<Cmp::HolyWellMultiBlock, Cmp::HolyWellSegment>( getReg(), random_origin_position.position, ms );
  }
  else if ( ms.get_sprite_type() == "RUIN.exterior_building" )
  {
    Factory::add_multiblock_with_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinSegment>( getReg(), random_origin_position.position, ms );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported multisprite type: {}", ms.get_sprite_type() );
    return;
  }
}

void RandomLevelGenerator::gen_crypt_interior_multiblocks()
{
  for ( int i = 0; i < 30; i++ )
  {
    auto [ms_type, ms_idx] = m_sprite_factory.get_random_type_and_texture_index( { "CRYPT.interior_mb1x3" } );
    auto &ms = m_sprite_factory.get_multisprite_by_type( ms_type );
    auto [random_entity, random_origin_position] = find_spawn_location( ms, 0 );
    if ( random_entity == entt::null )
    {
      SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
      return;
    }
    Factory::add_multiblock_with_segments<Cmp::CryptInteriorMultiBlock, Cmp::CryptInteriorSegment>( getReg(), random_origin_position.position, ms );
  }
}

std::pair<entt::entity, Cmp::Position> RandomLevelGenerator::find_spawn_location( const Sprites::MultiSprite &ms, unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        getReg(), Utils::Rnd::IncludePack<>{}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::PlayerCharacter>{}, current_seed );

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

      for ( auto [entity, holywell_cmp, holywell_pos_cmp] : getReg().view<Cmp::HolyWellSegment, Cmp::Position>().each() )
      {
        if ( holywell_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, ruin_cmp, ruin_pos_cmp] : getReg().view<Cmp::RuinSegment, Cmp::Position>().each() )
      {
        if ( ruin_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : getReg().view<Cmp::CryptObjectiveSegment, Cmp::Position>().each() )
      {
        if ( crypt_obj_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
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
      for ( auto [entity, player_cmp, player_pos_cmp] : getReg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
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
      auto new_entt = getReg().create();
      return { new_entt, getReg().emplace_or_replace<Cmp::Position>( new_entt, random_pos.position, random_pos.size ) };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

std::vector<entt::entity> RandomLevelGenerator::gen_random_plants( sf::Vector2u map_grid_size )
{
  std::vector<entt::entity> assigned_entts;

  auto num_plants = map_grid_size.x * map_grid_size.y / 200;

  for ( std::size_t i = 0; i < num_plants; ++i )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        getReg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // select a random number within the range of possible flora CarryItems
    auto [rand_plant_type, rnd_plant_idx] = m_sprite_factory.get_random_type_and_texture_index(
        { "CARRYITEM.plant1", "CARRYITEM.plant2", "CARRYITEM.plant3", "CARRYITEM.plant4", "CARRYITEM.plant5", "CARRYITEM.plant6", "CARRYITEM.plant7",
          "CARRYITEM.plant8", "CARRYITEM.plant9", "CARRYITEM.plant10", "CARRYITEM.plant11", "CARRYITEM.plant12" } );

    auto world_pos_entt = Utils::get_world_pos_entt( getReg(), random_pos );
    if ( world_pos_entt != entt::null )
    {
      // make sure we mark the *world* entt as reserved
      getReg().emplace_or_replace<Cmp::ReservedPosition>( world_pos_entt );

      // now create the plant at a new entt
      Factory::create_plant_obstacle( getReg(), random_pos, m_sprite_factory.get_multisprite_by_type( rand_plant_type ) );
      SPDLOG_DEBUG( "Created plant at {},{}", random_pos.position.x, random_pos.position.y );
      assigned_entts.push_back( random_entity );
    }
  }
  return assigned_entts;
}

} // namespace ProceduralMaze::Sys::ProcGen