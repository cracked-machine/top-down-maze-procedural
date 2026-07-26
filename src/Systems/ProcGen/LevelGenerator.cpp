#include <Components/Altar/AltarMultiBlock.hpp>
#include <Components/Altar/AltarSegment.hpp>
#include <Components/Crypt/CryptBuildingMultiBlock.hpp>
#include <Components/Crypt/CryptBuildingSegment.hpp>
#include <Components/Crypt/CryptInteriorMultiBlock.hpp>
#include <Components/Crypt/CryptInteriorSegment.hpp>
#include <Components/Crypt/CryptObjectiveMultiBlock.hpp>
#include <Components/Crypt/CryptObjectiveSegment.hpp>
#include <Components/Crypt/CryptRoomClosed.hpp>
#include <Components/Crypt/CryptRoomEnd.hpp>
#include <Components/Crypt/CryptRoomOpen.hpp>
#include <Components/Crypt/CryptRoomStart.hpp>
#include <Components/Exit.hpp>
#include <Components/Grave/GraveMultiBlock.hpp>
#include <Components/Grave/GraveSegment.hpp>
#include <Components/Grave/PlantMultiBlock.hpp>
#include <Components/Grave/PlantSegment.hpp>
#include <Components/Moveable.hpp>
#include <Components/Persistent/GraveNumMultiplier.hpp>
#include <Components/Persistent/MaxNumAltars.hpp>
#include <Components/Persistent/MaxNumCrypts.hpp>
#include <Components/Player/PlayerCharacter.hpp>
#include <Components/Position.hpp>
#include <Components/RectBounds.hpp>
#include <Components/ReservedPosition.hpp>
#include <Components/Ruin/RuinBuildingSegment.hpp>
#include <Components/Ruin/RuinCobweb.hpp>
#include <Components/Ruin/RuinGateSegment.hpp>
#include <Components/Ruin/RuinHexagramMultiBlock.hpp>
#include <Components/Ruin/RuinHexagramSegment.hpp>
#include <Components/Ruin/RuinStairsBalustradeMultiBlock.hpp>
#include <Components/Ruin/RuinStairsGateMultiBlock.hpp>
#include <Components/Ruin/RuinStairsLowerMultiBlock.hpp>
#include <Components/Ruin/RuinStairsUpperMultiBlock.hpp>
#include <Components/Ruin/RuneMarking.hpp>
#include <Components/SpawnArea.hpp>
#include <Components/Spring/HealingSpringBuildingMultiBlock.hpp>
#include <Components/Spring/HealingSpringBuildingSegment.hpp>
#include <Components/Spring/HealingSpringMultiBlock.hpp>
#include <Components/Spring/HealingSpringSegment.hpp>
#include <Components/Wall.hpp>
#include <Components/ZOrderValue.hpp>
#include <Events/CreateItemEvent.hpp>
#include <Factory/CryptFactory.hpp>
#include <Factory/MultiblockFactory.hpp>
#include <Factory/NpcFactory.hpp>
#include <Factory/ObstacleFactory.hpp>
#include <Factory/PlantFactory.hpp>
#include <Factory/PlayerFactory.hpp>
#include <Factory/RuinFactory.hpp>
#include <Factory/WallFactory.hpp>
#include <PathFinding/SpatialHashGrid.hpp>
#include <SceneControl/SceneData.hpp>
#include <Sprites/SpriteSheet.hpp>
#include <Systems/BaseSystem.hpp>
#include <Systems/PersistSystem.hpp>
#include <Systems/ProcGen/LevelGenerator.hpp>
#include <Systems/Render/RenderSystem.hpp>
#include <Utils/Collision.hpp>
#include <Utils/Constants.hpp>
#include <Utils/Optimizations.hpp>
#include <Utils/Player.hpp>
#include <Utils/Random.hpp>
#include <Utils/Utils.hpp>

#include <SFML/System/Vector2.hpp>
#include <ranges>
#include <spdlog/spdlog.h>

namespace Game::Sys::ProcGen
{

LevelGenerator::LevelGenerator( entt::registry &reg, sf::RenderWindow &window, Sprites::SpriteFactory &sprite_factory, Audio::SoundBank &sound_bank )
    : BaseSystem( reg, window, sprite_factory, sound_bank ),
      m_obstacle_sm( std::make_unique<PathFinding::SpatialHashGrid>() ),
      m_void_sm( std::make_unique<PathFinding::SpatialHashGrid>() ),
      m_non_obstacle_sm( std::make_unique<PathFinding::SpatialHashGrid>() )
{
}

PathFinding::SpatialHashGrid &LevelGenerator::get_obstacle_sm() { return *m_obstacle_sm; }
PathFinding::SpatialHashGrid &LevelGenerator::get_void_sm() { return *m_void_sm; }
PathFinding::SpatialHashGrid &LevelGenerator::get_non_obstacle_sm() { return *m_non_obstacle_sm; }

void LevelGenerator::build_scene_from_data( const Scene::SceneData &scene_data )
{
  auto [map_size_grid, map_size_pixel] = scene_data.map_size();
  auto w = map_size_grid.x;

  // Walls
  [[maybe_unused]] const Sprites::SpriteSheet &wall_ms = m_sprite_factory.get_spritesheet_by_type( scene_data.wall_tileset().name );
  for ( const auto [i, tile] : std::views::enumerate( scene_data.wall_tilelayer() ) )
  {
    auto row = i / w; // increments every 'w' tiles
    auto col = i % w; // wraps back to zero every 'w' tiles
    sf::Vector2f new_pos( static_cast<float>( col ) * Constants::kGridSizePxF.x, static_cast<float>( row ) * Constants::kGridSizePxF.y );
    if ( tile >= scene_data.wall_tileset().first_gid )
    {
      // get the relative index using the offset from the json data
      Factory::add_wall_entity( reg(), new_pos, wall_ms, tile - scene_data.wall_tileset().first_gid );
    }
  }

  // Levelgen layer
  for ( const auto [i, tile] : std::views::enumerate( scene_data.levelgen_tilelayer() ) )
  {
    auto col = i % w; // wraps back to zero every 'w' tiles
    auto row = i / w; // increments every 'w' tiles
    sf::Vector2f new_pos( static_cast<float>( col ) * Constants::kGridSizePxF.x, static_cast<float>( row ) * Constants::kGridSizePxF.y );
    if ( tile == scene_data.void_tile_id() )
    {
      Cmp::Position new_pos_cmp( new_pos, Constants::kGridSizePxF );
      auto entt = Factory::create_void_pos( reg(), new_pos_cmp );
      m_void_sm->insert( entt, new_pos_cmp );
    }
    else if ( tile == scene_data.open_tile_id() ) { Factory::create_world_pos( reg(), new_pos ); }
    else if ( tile == scene_data.spawn_tile_id() )
    {
      auto entity = Factory::create_world_pos( reg(), new_pos );
      Factory::add_spawn_area( reg(), entity, m_sprite_factory, new_pos.y - 16.0f );
    }
    else if ( tile == scene_data.exit_tile_id() ) { Crypt::Factory::create_crypt_exit( reg(), new_pos ); }
    else if ( tile == scene_data.reserved_tile_id() ) { Factory::add_reservedposition( reg(), new_pos ); }
  }

  // Solid Layer
  for ( const auto &solid : scene_data.solid_objectlayer() )
  {
    Factory::add_solid_player( reg(), solid );
    Factory::add_solid_npc( reg(), solid );
    Factory::add_no_move_dest( reg(), solid );
  }

  // Multiblock layer
  for ( const auto &[ms_type, pos] : scene_data.multiblock_objectlayer() )
  {
    const auto &ms = m_sprite_factory.get_spritesheet_by_type( ms_type );
    if ( ms_type == "sprite.graveyard.healingspring" )
    {
      Factory::add_multiblock_with_segments<Cmp::HealingSpringMultiBlock, Cmp::HealingSpringSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.crypt.objective.closed" )
    {
      Factory::add_multiblock_with_segments<Cmp::CryptObjectiveMultiBlock, Cmp::CryptObjectiveSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.crypt.altar.inactive" )
    {
      Factory::add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "npc.drknox" )
    {
      auto npc_entt = reg().create();
      reg().emplace_or_replace<Cmp::Position>( npc_entt, pos, Constants::kGridSizePxF );
      Factory::create_npc( m_reg, npc_entt, ms_type );
    }
    else if ( ms_type == "sprite.ruin.stairs.up" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsLowerMultiBlock, Cmp::RuinStairsSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.ruin.stairs.down" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsUpperMultiBlock, Cmp::RuinStairsSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.ruin.stairs.balustrade" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsBalustradeMultiBlock, Cmp::RuinStairsSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.ruin.stairs.gate" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinStairsGateMultiBlock, Cmp::RuinGateSegment>( reg(), pos, ms );
    }
    else if ( ms_type == "sprite.ruin.hex" )
    {
      Factory::add_multiblock_with_segments<Cmp::RuinHexagramMultiBlock, Cmp::RuinHexagramSegment>( m_reg, pos, ms );
    }
    else if ( ms_type.contains( "item." ) )
    {
      auto [inventory_entt, inventory_type] = Utils::Player::get_inventory_type( reg() );
      // prevent infinite respawns in the RuinSceneUpperFloor
      if ( inventory_type == "item.witchesjar" ) continue;

      // make sure we mark the *world* entt as reserved
      auto world_pos_entt = Utils::get_world_pos_entt( m_reg, Cmp::Position( pos, ms.get_sprite_size() ) );
      if ( world_pos_entt != entt::null )
      {
        reg().emplace_or_replace<Cmp::ReservedPosition>( world_pos_entt );
        get_systems_event_queue().trigger( Events::CreateItemEvent( Cmp::Position( pos, ms.get_sprite_size() ), ms_type ) );
      }
    }
  }
}

void LevelGenerator::add_graveyard_exterior_obstacles( float init_chance, PathFinding::SpatialHashGridSharedPtr reserved_navmesh )
{
  auto position_view = reg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    if ( Cmp::RandomFloat{ 0.f, 1.f }.gen() < init_chance )
    {
      if ( Factory::add_obstacle( reg(), entity, reserved_navmesh ) )
      {
        m_obstacle_sm->insert( entity, pos_cmp );
        reserved_navmesh->insert( entity, pos_cmp );
      }
    }
  }
}

void LevelGenerator::decorate_graveyard_exterior_obstacles()
{
  const Sprites::SpriteSheet &ss_main = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.wall.int.main" );
  const Sprites::SpriteSheet &ss_cap = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.wall.int.cap" );

  auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
  {
    auto uuid = Cmp::UUID::generate();
    auto [_, rand_obst_tex_idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.graveyard.wall.int.main" } );

    Factory::decorate_obstacle( reg(), obstacle_entt, obstacle_pos_cmp, ss_main, rand_obst_tex_idx );
    reg().emplace_or_replace<Cmp::UUID>( obstacle_entt, uuid );

    auto cap_entt = reg().create();
    Cmp::Position cap_position( { obstacle_pos_cmp.x(), obstacle_pos_cmp.y() - obstacle_pos_cmp.size.y }, obstacle_pos_cmp.size );
    reg().emplace_or_replace<Cmp::Position>( cap_entt, cap_position );
    Factory::decorate_obstacle( reg(), cap_entt, cap_position, ss_cap, rand_obst_tex_idx, obstacle_pos_cmp.y() + 1, false );
    reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
  }
}

void LevelGenerator::add_ruin_interior_obstacles( float init_chance, PathFinding::SpatialHashGridSharedPtr reserved_navmesh )
{
  auto position_view = reg().view<Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Exit> );
  for ( auto [entity, pos_cmp] : position_view.each() )
  {
    if ( reserved_navmesh->at( pos_cmp ).empty() )
    {
      if ( Cmp::RandomFloat{ 0.f, 1.f }.gen() < init_chance )
      {
        Factory::add_obstacle( reg(), entity );
        m_obstacle_sm->insert( entity, pos_cmp );
        reserved_navmesh->insert( entity, pos_cmp );
      }
    }
  }
}

void LevelGenerator::decorate_ruin_interior_obstacles()
{
  const Sprites::SpriteSheet &ss_main = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.wall.int.main" );
  const Sprites::SpriteSheet &ss_cap = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.wall.int.cap" );
  constexpr int ss_main_idx = 0;
  constexpr int ss_cap_idx = 0;

  auto obstacle_view = reg().view<Cmp::Obstacle, Cmp::Position>( entt::exclude<Cmp::PlayerCharacter, Cmp::ReservedPosition> );
  for ( auto [obstacle_entt, obstacle_cmp, obstacle_pos_cmp] : obstacle_view.each() )
  {
    auto uuid = Cmp::UUID::generate();
    Factory::decorate_obstacle( reg(), obstacle_entt, obstacle_pos_cmp, ss_main, ss_main_idx );
    reg().emplace_or_replace<Cmp::UUID>( obstacle_entt, uuid );
    reg().emplace_or_replace<Cmp::Moveable>( obstacle_entt );

    auto cap_entt = reg().create();
    Cmp::Position cap_position( { obstacle_pos_cmp.x(), obstacle_pos_cmp.y() - obstacle_pos_cmp.size.y }, obstacle_pos_cmp.size );
    reg().emplace_or_replace<Cmp::Position>( cap_entt, cap_position );
    Factory::decorate_obstacle( reg(), cap_entt, cap_position, ss_cap, ss_cap_idx, obstacle_pos_cmp.y(), false );
    reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
    reg().emplace_or_replace<Cmp::UUID>( cap_entt, uuid );
  }
}

void LevelGenerator::add_ruin_rune_markers()
{
  for ( auto _ : std::views::iota( 0, 5 ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( reg(), {}, Utils::Rnd::ExcludePack<Cmp::ReservedPosition>{} );
    auto [_, idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.ruin.runemarking.inactive" } );

    auto rune_entt = reg().create();
    reg().emplace_or_replace<Cmp::ReservedPosition>( rune_entt );
    reg().emplace_or_replace<Cmp::Position>( rune_entt, rnd_pos );
    float zorder = m_sprite_factory.get_spritesheet_by_type( "sprite.ruin.runemarking.inactive" ).get_zorder( 0 );
    reg().emplace_or_replace<Cmp::ZOrderValue>( rune_entt, zorder );
    reg().emplace_or_replace<Cmp::RuneMarking>( rune_entt );
    // clang-format off
    reg().emplace_or_replace<Cmp::AnimData>( rune_entt, Cmp::AnimData::Config{ 
          .sprite_type = "sprite.graveyard.playerspawn", 
          .frame_index_offset = idx,
          .enabled = true
    });
    // clang-format on

    SPDLOG_INFO( "Added rune to {},{}", rnd_pos.x(), rnd_pos.y() );

    // set the existing entt to reserved as well
    for ( auto [world_entt, world_pos_cmp] : reg().view<Cmp::Position>().each() )
    {
      if ( world_pos_cmp.findIntersection( rnd_pos ) ) reg().emplace_or_replace<Cmp::ReservedPosition>( rnd_entt );
    }
    m_non_obstacle_sm->insert( rune_entt, rnd_pos );
  }
}

void LevelGenerator::add_lowerfloor_cobwebs( int num_cobwebs, sf::FloatRect scene_dimensions )
{
  auto has_collision = [&]( const Cmp::RectBounds &pos )
  {
    if ( Utils::Collision::check_cmp<Cmp::RuinStairsLowerMultiBlock>( reg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::RuinCobweb>( reg(), pos ) ) { return true; }
    if ( Utils::Collision::check_cmp<Cmp::Exit>( reg(), pos ) ) { return true; }

    // ensure is inside scene
    if ( not Cmp::RectBounds::scaled( pos.position(), pos.size(), 1.5f ).findIntersection( scene_dimensions ) ) { return true; }
    return false;
  };

  constexpr auto gridsize = Constants::kGridSizePxF;
  int max_cobwebs = num_cobwebs;
  for ( auto _ : std::views::iota( 0, max_cobwebs ) )
  {
    auto [rnd_entt, rnd_pos] = Utils::Rnd::get_random_position( reg(), {}, Utils::Rnd::ExcludePack<Cmp::ReservedPosition>{} );
    if ( rnd_entt == entt::null ) continue;

    if ( has_collision( Cmp::RectBounds::scaled( { rnd_pos.position }, gridsize, 1 ) ) ) continue;
    auto [ms, idx] = m_sprite_factory.get_random_type_and_texture_index( { "sprite.ruin.cobweb" } );
    Factory::create_cobweb( reg(), rnd_entt, rnd_pos.position, m_sprite_factory.get_spritesheet_by_type( ms ), idx );
    m_non_obstacle_sm->insert( rnd_entt, rnd_pos );
  }
}

void LevelGenerator::gen_graveyard_exterior_multiblocks()
{
  auto grave_num_multiplier = Sys::PersistSystem::get<Cmp::Persist::GraveNumMultiplier>( reg() );
  auto max_num_altars = Sys::PersistSystem::get<Cmp::Persist::MaxNumAltars>( reg() );
  auto max_num_crypts = Sys::PersistSystem::get<Cmp::Persist::MaxNumCrypts>( reg() );
  std::size_t max_number_healing_springs = 1;
  std::size_t max_number_ruins = 1;

  // GRAVES
  auto grave_meta_types = m_sprite_factory.get_all_sprite_types_by_pattern( R"(graves\.\w+\.closed$)" );
  if ( grave_meta_types.empty() ) { SPDLOG_WARN( "No GRAVE spritesheets found in SpriteFactory" ); }
  else
  {
    SPDLOG_DEBUG( "Found {}, {}", grave_meta_types[0], grave_meta_types[1] );
    uint8_t max_num_graves = max_num_altars.get_value() * grave_num_multiplier.get_value();
    for ( std::size_t i = 0; i < max_num_graves; ++i )
    {
      auto [sprite_metatype, unused_index] = m_sprite_factory.get_random_type_and_texture_index( grave_meta_types );
      SPDLOG_DEBUG( "Selected {}, {}", sprite_metatype, unused_index );
      const auto &spritesheet = m_sprite_factory.get_spritesheet_by_type( sprite_metatype );
      do_gen_graveyard_exterior_multiblock( spritesheet, unused_index );
    }
  }

  // ALTARS
  const auto &altar_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.altar.inactive" );
  for ( std::size_t i = 0; i < max_num_altars.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( altar_spritesheet, 0 );
  }

  // CRYPTS - note: we use keys from altars to open crypts so the number should be equal
  const auto &crypt_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.crypt.closed" );
  for ( std::size_t i = 0; i < max_num_crypts.get_value(); ++i )
  {
    do_gen_graveyard_exterior_multiblock( crypt_spritesheet, 0 );
  }

  const auto &healingspring_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.building.healingspring" );
  for ( std::size_t i = 0; i < max_number_healing_springs; ++i )
  {
    do_gen_graveyard_exterior_multiblock( healingspring_spritesheet, 0 );
  }

  const auto &ruin_spritesheet = m_sprite_factory.get_spritesheet_by_type( "sprite.graveyard.ruin" );
  for ( std::size_t i = 0; i < max_number_ruins; ++i )
  {
    do_gen_graveyard_exterior_multiblock( ruin_spritesheet, 0 );
  }
}

void LevelGenerator::do_gen_graveyard_exterior_multiblock( const Sprites::SpriteSheet &ms, size_t ms_index, unsigned long seed )
{
  auto [random_entity, random_origin_position] = find_spawn_location( ms, seed );
  if ( random_entity == entt::null )
  {
    SPDLOG_ERROR( "Failed to find valid spawn position for {}.", ms.get_sprite_type() );
    return;
  }

  if ( ms.get_sprite_type().contains( "altar" ) )
  {
    Factory::add_multiblock_with_segments<Cmp::AltarMultiBlock, Cmp::AltarSegment>( reg(), random_origin_position.position, ms );
  }
  else if ( ms.get_sprite_type().contains( "graves" ) )
  {
    Factory::add_multiblock_with_segments<Cmp::GraveMultiBlock, Cmp::GraveSegment>( reg(), random_origin_position.position, ms, ms_index );
  }
  else if ( ms.get_sprite_type() == "sprite.graveyard.crypt.closed" )
  {
    Factory::add_multiblock_with_segments<Cmp::CryptBuildingMultiBlock, Cmp::CryptBuildingSegment>( reg(), random_origin_position.position, ms );
    SPDLOG_INFO( "Added {} to {},{}", ms.get_sprite_type(), random_origin_position.position.x, random_origin_position.position.y );
  }
  else if ( ms.get_sprite_type() == "sprite.graveyard.building.healingspring" )
  {
    Factory::add_multiblock_with_segments<Cmp::HealingSpringBuildingMultiBlock, Cmp::HealingSpringBuildingSegment>(
        reg(), random_origin_position.position, ms );
    SPDLOG_INFO( "Added {} to {},{}", ms.get_sprite_type(), random_origin_position.position.x, random_origin_position.position.y );
  }
  else if ( ms.get_sprite_type() == "sprite.graveyard.ruin" )
  {
    Factory::add_multiblock_with_segments<Cmp::RuinBuildingMultiBlock, Cmp::RuinBuildingSegment>( reg(), random_origin_position.position, ms );
    SPDLOG_INFO( "Added {} to {},{}", ms.get_sprite_type(), random_origin_position.position.x, random_origin_position.position.y );
  }
  else
  {
    SPDLOG_ERROR( "gen_large_obstacle called with unsupported spritesheet type: {}", ms.get_sprite_type() );
    return;
  }
}

std::pair<entt::entity, Cmp::Position> LevelGenerator::find_spawn_location( const Sprites::SpriteSheet &ms, unsigned long seed )
{
  constexpr int kMaxAttempts = 1000;
  int attempts = 0;
  unsigned long current_seed = seed;

  while ( attempts < kMaxAttempts )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        reg(), Utils::Rnd::IncludePack<>{}, Utils::Rnd::ExcludePack<Cmp::Wall, Cmp::ReservedPosition, Cmp::PlayerCharacter>{}, current_seed );

    auto lo_sprite_size = m_sprite_factory.get_sprite_size_by_type( ms.get_sprite_type() );
    auto new_lo_hitbox = Cmp::RectBounds::scaled( random_pos.position, lo_sprite_size, 1.f );

    // Check collisions with walls, graves, shrines
    auto is_valid = [&]() -> bool
    {
      // return false for wall collisions
      for ( auto [entity, wall_cmp, wall_pos_cmp] : reg().view<Cmp::Wall, Cmp::Position>().each() )
      {
        if ( wall_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for grave collisions
      for ( auto [entity, grave_cmp, grave_pos_cmp] : reg().view<Cmp::GraveSegment, Cmp::Position>().each() )
      {
        if ( grave_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for altar collisions
      for ( auto [entity, altar_cmp, altar_pos_cmp] : reg().view<Cmp::AltarSegment, Cmp::Position>().each() )
      {
        if ( altar_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for crypt collisions
      for ( auto [entity, crypt_cmp, crypt_pos_cmp] : reg().view<Cmp::CryptBuildingSegment, Cmp::Position>().each() )
      {
        if ( crypt_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, spring_cmp, spring_pos_cmp] : reg().view<Cmp::HealingSpringBuildingSegment, Cmp::Position>().each() )
      {
        if ( spring_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, ruin_cmp, ruin_pos_cmp] : reg().view<Cmp::RuinBuildingSegment, Cmp::Position>().each() )
      {
        if ( ruin_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      for ( auto [entity, crypt_obj_cmp, crypt_obj_pos_cmp] : reg().view<Cmp::CryptObjectiveSegment, Cmp::Position>().each() )
      {
        if ( crypt_obj_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for reserved position collisions
      for ( auto [entity, reserved_cmp, reserved_pos_cmp] : reg().view<Cmp::ReservedPosition, Cmp::Position>().each() )
      {
        if ( reserved_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for spawn area collisions
      for ( auto [entity, spawn_cmp, spawn_pos_cmp] : reg().view<Cmp::SpawnArea, Cmp::Position>().each() )
      {
        if ( spawn_pos_cmp.findIntersection( new_lo_hitbox.getBounds() ) ) return false;
      }

      // Return false for playable character collisions
      for ( auto [entity, player_cmp, player_pos_cmp] : reg().view<Cmp::PlayerCharacter, Cmp::Position>().each() )
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
      auto new_entt = reg().create();
      return { new_entt, reg().emplace_or_replace<Cmp::Position>( new_entt, random_pos.position, random_pos.size ) };
    }

    attempts++;
    // Increment seed for next attempt (works for both seeded and non-seeded cases)
    if ( seed > 0 ) { current_seed++; }
  }

  SPDLOG_ERROR( "Failed to find valid large obstacle spawn location after {} attempts (original seed: {})", kMaxAttempts, seed );
  return { entt::null, Cmp::Position{ { 0.f, 0.f }, { 0.f, 0.f } } };
}

std::vector<entt::entity> LevelGenerator::gen_random_plants( sf::Vector2u map_grid_size, PathFinding::SpatialHashGridSharedPtr reserved_navmesh )
{
  std::vector<entt::entity> assigned_entts;

  auto num_plants = map_grid_size.x * map_grid_size.y / 200;

  for ( std::size_t i = 0; i < num_plants; ++i )
  {
    auto [random_entity, random_pos] = Utils::Rnd::get_random_position(
        reg(), {}, Utils::Rnd::ExcludePack<Cmp::PlayerCharacter, Cmp::ReservedPosition, Cmp::Obstacle>{}, 0 );

    // select a random number within the range of possible flora CarryItems
    auto [rand_plant_type, rnd_plant_idx] = m_sprite_factory.get_random_type_and_texture_index(
        { "sprite.item.plant1", "sprite.item.plant2", "sprite.item.plant3", "sprite.item.plant4", "sprite.item.plant5", "sprite.item.plant6",
          "sprite.item.plant7", "sprite.item.plant8", "sprite.item.plant9", "sprite.item.plant10", "sprite.item.plant11", "sprite.item.plant12" } );

    auto world_pos_entt = Utils::get_world_pos_entt( reg(), random_pos );
    if ( world_pos_entt != entt::null )
    {
      if ( reserved_navmesh->at( random_pos ).empty() )
      {
        // now create the plant at a new entt
        auto [mb_entt, _] = Factory::add_multiblock_with_segments<Cmp::PlantMultiBlock, Cmp::PlantSegment>(
            reg(), random_pos.position, m_sprite_factory.get_spritesheet_by_type( rand_plant_type ) );
        SPDLOG_DEBUG( "Created plant at {},{}", random_pos.position.x, random_pos.position.y );
        assigned_entts.push_back( random_entity );

        // Protect every cell covered by this plant: segment entities carry the UUID and
        // are at each tile position. Insert them so subsequent placement calls see them
        // as reserved via the O(1) navmesh check.
        auto *mb_uuid = reg().try_get<Cmp::UUID>( mb_entt );
        if ( mb_uuid )
        {
          for ( auto [seg_entt, seg_cmp, seg_pos, seg_uuid] : reg().view<Cmp::PlantSegment, Cmp::Position, Cmp::UUID>().each() )
          {
            if ( seg_uuid == *mb_uuid ) reserved_navmesh->insert( seg_entt, seg_pos );
          }
        }
      }
    }
  }
  return assigned_entts;
}

void LevelGenerator::reset()
{
  m_obstacle_sm->clear();
  m_void_sm->clear();
  m_non_obstacle_sm->clear();
}

} // namespace Game::Sys::ProcGen